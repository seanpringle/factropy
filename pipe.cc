#include "common.h"
#include "pipe.h"

void Pipe::reset() {
	all.clear();
}

void Pipe::tick() {
	PipeNetwork::tick();
}

Pipe& Pipe::create(uint id) {
	Pipe& pipe = all.ref(id);
	pipe.id = id;
	pipe.network = NULL;
	pipe.cacheFid = 0;
	pipe.cacheTally = 0;
	return pipe;
}

Pipe& Pipe::get(uint id) {
	ensuref(all.has(id), "invalid pipe access %d", id);
	return all.ref(id);
}

void Pipe::destroy() {
	all.drop(id);
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

std::unordered_set<uint> Pipe::servicing(Box box) {
	std::unordered_set<uint> hits;
	for (uint id: Entity::intersecting(box.grow(0.5f))) {
		if (hits.count(id)) continue;
		Entity& en = Entity::get(id);
		if (!en.spec->pipe) continue;
		Pipe& pipe = en.pipe();
		// only pipes adjacent to the box, pointing inward
		if (en.box().intersects(box.shrink(0.1f))) continue;

		for (Point point: pipe.pipeConnections()) {
			if (box.intersects(point.box().grow(0.1f))) {
				hits.insert(pipe.id);
			}
		}
	}
	return hits;
}

void PipeNetwork::tick() {
	if (rebuild) {

		while (all.size()) {
			delete *(all.begin());
		}

		std::function<void(uint)> flood = [&](uint pid) {
			Entity& en = Entity::get(pid);
			if (en.isGhost()) return;

			Pipe& pipe = Pipe::get(pid);

			for (Point p: pipe.pipeConnections()) {
				for (uint sid: Entity::intersecting(p.box().grow(0.1))) {
					if (sid == pipe.id) continue;

					Entity& sen = Entity::get(sid);
					if (sen.isGhost()) continue;
					if (!sen.spec->pipe) continue;

					Pipe& other = Pipe::get(sid);

					bool join = false;
					Box box = p.box().grow(0.1f);

					for (Point op: other.pipeConnections()) {
						if (box.intersects(op.box().grow(0.1f))) {
							join = true;
							break;
						}
					}

					if (join && !other.network) {
						other.network = pipe.network;
						other.network->pipes.insert(other.id);
						flood(sid);
					}
				}
			}
		};

		for (Pipe& pipe: Pipe::all) {
			Entity& en = Entity::get(pipe.id);
			if (en.isGhost()) continue;

			if (pipe.cacheFid && !pipe.network) {
				pipe.network = new PipeNetwork();
				pipe.network->pipes.insert(pipe.id);
				flood(pipe.id);
			}
		}

		for (Pipe& pipe: Pipe::all) {
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
					network->tally = pipe.cacheTally;
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
