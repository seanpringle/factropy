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

		std::vector<uint> pipes;
		for (Pipe& pipe: Pipe::all) {
			Entity& en = Entity::get(pipe.id);
			if (en.isGhost()) continue;
			ensure(!pipe.network);
			pipes.push_back(pipe.id);
		}

		while (pipes.size()) {
			Pipe& pipe = Pipe::get(pipes.back());
			pipes.pop_back();

			if (pipe.network) continue;

			for (Point p: pipe.pipeConnections()) {
				for (uint sid: Entity::intersecting(p.box().grow(0.1))) {
					if (sid == pipe.id) continue;

					Entity& sen = Entity::get(sid);
					if (sen.isGhost()) continue;
					if (!sen.spec->pipe) continue;

					Pipe& other = sen.pipe();

					// ensure new networks propagate flood-fill style
					if (!other.network) {
						pipes.push_back(sid);
					}

					// propagate first valid network from a sibling
					if (!pipe.network && other.network && (!other.network->fid || other.network->fid == pipe.cacheFid)) {
						for (Point s: other.pipeConnections()) {
							if (s.box().grow(0.1).intersects(p.box().grow(0.1))) {
								pipe.network = other.network;
								pipe.network->pipes.insert(pipe.id);
								if (!other.network->fid) {
									other.network->fid = pipe.cacheFid;
								}
								break;
							}
						}
					}
				}
			}

			if (!pipe.network) {
				pipe.network = new PipeNetwork();
				pipe.network->pipes.insert(pipe.id);
				pipe.network->fid = pipe.cacheFid;
			}
		}

		for (auto network: all) {
			for (uint id: network->pipes) {
				Entity& en = Entity::get(id);
				Pipe& pipe = Pipe::get(id);
				pipe.cacheFid = network->fid;
				network->limit += en.spec->pipeCapacity;
				network->tally += pipe.cacheTally;
			}
		}

		rebuild = false;
	}
}

PipeNetwork::PipeNetwork() {
	all.insert(this);
	limit = 0;
	tally = 0;
}

PipeNetwork::~PipeNetwork() {
	float fill = level();
	for (uint id: pipes) {
		Entity& en = Entity::get(id);
		Pipe& pipe = Pipe::get(id);
		pipe.cacheFid = fid;
		pipe.cacheTally = fid ? (uint)std::ceil((float)en.spec->pipeCapacity.fluids(fid) * fill): 0;
		pipe.network = NULL;
	}
	pipes.clear();
	all.erase(this);
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
		uint count = std::min(tally, amount.size);
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
	return fid ? (Fluid::get(fid)->liquid * tally).portion(limit): 0.0f;
}