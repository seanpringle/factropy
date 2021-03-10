#include "common.h"
#include "pipe.h"

void Pipe::reset() {
	all.clear();
}

void Pipe::tick() {
	PipeNetwork::tick();
}

Pipe& Pipe::create(uint id) {
	Entity& en = Entity::get(id);
	Pipe& pipe = all[id];
	pipe.id = id;
	pipe.network = NULL;
	pipe.cacheFid = 0;
	pipe.cacheTally = 0;
	pipe.partner = 0;
	pipe.underground = en.spec->pipeUnderground;
	return pipe;
}

Pipe& Pipe::get(uint id) {
	return all.refer(id);
}

void Pipe::destroy() {
	all.erase(id);
}

void Pipe::manage() {
	ensure(!network);
	PipeNetwork::rebuild = true;
}

void Pipe::unmanage() {
	ensure(network);
	network->pipes.erase(id);
	network = NULL;
	PipeNetwork::rebuild = true;
}

std::vector<Point> Pipe::pipeConnections() {
	Entity& en = Entity::get(id);
	return en.spec->relativePoints(en.spec->pipeConnections, en.dir.rotation(), en.pos);
}

std::vector<uint> Pipe::servicing(Box box) {
	std::vector<uint> hits;
	for (uint id: Entity::intersecting(box.grow(0.5f))) {
		Entity& en = Entity::get(id);
		if (!en.spec->pipe) continue;
		Pipe& pipe = en.pipe();
		// only pipes adjacent to the box, pointing inward
		if (en.box().intersects(box.shrink(0.1f))) continue;

		for (Point point: pipe.pipeConnections()) {
			if (box.intersects(point.box().grow(0.1f))) {
				hits.push_back(pipe.id);
			}
		}
	}
	deduplicate(hits);
	return hits;
}

void PipeNetwork::tick() {
	if (rebuild) {

		while (all.size()) {
			delete *(all.begin());
		}

		for (auto& pipe: Pipe::all) {
			ensure(!pipe.network);
		}

		for (auto& pipe: Pipe::all) {
			if (!pipe.underground) continue;
			pipe.partner = 0;
		}

		for (auto& pipe: Pipe::all) {
			if (!pipe.underground) continue;
			Entity& en = Entity::get(pipe.id);
			if (en.isGhost()) continue;

			float dist = 0.0f;

			for (uint pid: Entity::intersecting(pipe.undergroundRange())) {
				if (pipe.id == pid) continue;
				if (!Pipe::all.has(pid)) continue;
				Entity& eo = Entity::get(pid);
				if (eo.isGhost()) continue;
				Pipe& other = Pipe::get(pid);
				if (!other.underground) continue;
				if (other.partner) continue;
				if (en.dir != eo.dir.oppositeCardinal()) continue;
				float d = en.pos.distance(eo.pos);
				if (!pipe.partner || d < dist) {
					pipe.partner = pid;
					dist = d;
				}
			}

			if (pipe.partner) {
				Pipe::get(pipe.partner).partner = pipe.id;
			}
		}

		std::function<void(uint)> flood = [&](uint pid) {
			Entity& en = Entity::get(pid);
			if (en.isGhost()) return;

			Pipe& pipe = Pipe::get(pid);

			for (Point p: pipe.pipeConnections()) {
				Box box = p.box().grow(0.1f);
				for (uint oid: Entity::intersecting(box)) {
					if (oid == pipe.id) continue;

					Entity& sen = Entity::get(oid);
					if (sen.isGhost()) continue;
					if (!sen.spec->pipe) continue;

					Pipe& other = Pipe::get(oid);
					bool join = false;

					for (Point op: other.pipeConnections()) {
						if (box.intersects(op.box().grow(0.1f))) {
							join = true;
							break;
						}
					}

					if (join) {
						//ensure(!other.network || other.network == pipe.network);
						if (!other.network) {
							other.network = pipe.network;
							pipe.network->pipes.insert(other.id);
							flood(other.id);
						}
					}
				}
			}

			if (pipe.partner) {
				Pipe& other = Pipe::get(pipe.partner);
				//ensure(!other.network || other.network == pipe.network);
				if (!other.network) {
					other.network = pipe.network;
					other.network->pipes.insert(other.id);
					flood(pipe.partner);
				}
			}
		};

		for (auto& pipe: Pipe::all) {
			Entity& en = Entity::get(pipe.id);
			if (en.isGhost()) continue;

			if (pipe.cacheFid && !pipe.network) {
				pipe.network = new PipeNetwork();
				pipe.network->pipes.insert(pipe.id);
				flood(pipe.id);
			}
		}

		for (auto& pipe: Pipe::all) {
			Entity& en = Entity::get(pipe.id);
			if (en.isGhost()) continue;

			if (!pipe.network) {
				pipe.network = new PipeNetwork();
				pipe.network->pipes.insert(pipe.id);
				flood(pipe.id);
			}
		}

		for (auto network: all) {
			for (uint id: network->pipes) {
				Pipe& pipe = Pipe::get(id);
				if (pipe.cacheFid) {
					network->fid = pipe.cacheFid;
					//network->tally = pipe.cacheTally;
					break;
				}
			}
		}

		for (auto network: all) {
			for (uint id: network->pipes) {
				Entity& en = Entity::get(id);
				Pipe& pipe = Pipe::get(id);
				network->limit += en.spec->pipeCapacity;
				if (pipe.cacheFid == network->fid) {
					network->tally += pipe.cacheTally;
				}
				pipe.cacheFid = network->fid;
				pipe.cacheTally = 0;
			}
			if (!network->tally) {
				network->fid = 0;
			}
			if (network->tally > network->limit.value) {
				network->tally = network->limit.value;
			}
		}


		rebuild = false;
	}
}

Amount Pipe::contents() {
	if (network && network->fid) {
		Entity& en = Entity::get(id);
		uint fid = network->fid;
		float fill = network->level();
		uint n = std::ceil((float)en.spec->pipeCapacity.fluids(fid) * fill);
		return {fid, n};
	}
	return {0,0};
}

Box Pipe::undergroundRange() {
	Entity& en = Entity::get(id);
	Point far = (en.dir * en.spec->pipeUndergroundRange) + en.pos;
	return Box(en.pos, far).grow(0.1f);
}

void Pipe::flush() {
	if (network) {
		network->flush();
	}
}

PipeNetwork::PipeNetwork() {
	all.insert(this);
	fid = 0;
	limit = 0;
	tally = 0;
}

PipeNetwork::~PipeNetwork() {
	cacheState();
	for (uint id: pipes) {
		Pipe& pipe = Pipe::get(id);
		pipe.network = NULL;
	}
	pipes.clear();
	all.erase(this);
}

void PipeNetwork::cacheState() {
	float fill = level();
	for (uint id: pipes) {
		Entity& en = Entity::get(id);
		Pipe& pipe = Pipe::get(id);
		pipe.cacheFid = fid;
		pipe.cacheTally = fid ? (uint)std::ceil((float)en.spec->pipeCapacity.fluids(fid) * fill): 0;
	}
}

Amount PipeNetwork::inject(Amount amount) {
	if (!fid) {
		fid = amount.fid;
	}
	if (amount.fid == fid) {
		uint count = std::min(space(fid), amount.size);
		amount.size -= count;
		tally += count;
	}
	return amount;
}

Amount PipeNetwork::extract(Amount amount) {
	if (amount.fid == fid) {
		uint count = std::min(tally, (int)amount.size);
		amount.size = count;
		tally -= count;
		return amount;
	}
	return {0,0};
}

uint PipeNetwork::count(uint ffid) {
	return fid == ffid ? tally: 0;
}

uint PipeNetwork::space(uint ffid) {
	if (!fid) return limit;
	return fid == ffid ? limit.fluids(fid)-tally: 0;
}

float PipeNetwork::level() {
	return fid ? Liquid(Fluid::get(fid)->liquid.value * tally).portion(limit): 0.0f;
}

void PipeNetwork::flush() {
	fid = 0;
	tally = 0;
}