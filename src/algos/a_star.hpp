#pragma once
#include "../includes.hpp"

struct a_star : solution_interface {
	ret_t solve(const maze_t& maze) {
		struct node_t {
			point_t pos;
			unsigned distance;
			unsigned f_cost;
			bool operator>(const node_t& other) const { return f_cost > other.f_cost; }
		};

		auto idx = [&maze](const point_t& p) { return p.y * maze.width + p.x; };

		std::priority_queue<node_t, std::vector<node_t>, std::greater<node_t>> priority_queue;
		std::vector<point_t> previous(maze.width * maze.height, { -1, -1 });
		std::vector<unsigned> distances(maze.width * maze.height, UINT_MAX);

		distances[idx(maze.start)] = 0;
		priority_queue.push({ maze.start, 0, (unsigned)abs(maze.start.x - maze.end.x) + (unsigned)abs(maze.start.y - maze.end.y) });

		auto completed = false;

		while (!priority_queue.empty()) {
			auto current = priority_queue.top();
			priority_queue.pop();

			if (current.distance > distances[idx(current.pos)])
				continue;

			if (!maze.grid[idx(current.pos)])
				continue;

			if (current.pos == maze.end) {
				completed = true;
				break;
			}

			for (const auto& v : current.pos.neighbours(maze.width, maze.height)) {
				if (!maze.grid[idx(v)]) continue;
				const auto new_distance = distances[idx(current.pos)] + 1;
				if (new_distance < distances[idx(v)]) {
					distances[idx(v)] = new_distance;
					previous[idx(v)] = current.pos;
					priority_queue.push({ v, new_distance, new_distance + abs(v.x - maze.end.x) + abs(v.y - maze.end.y) });
				}
			}
		}

		std::deque<point_t> path;
		auto current = maze.end;
		while (!(current == point_t{ -1, -1 })) {
			path.push_front(current);
			current = previous[idx(current)];
		}

		return { completed, distances, std::vector<point_t>(path.begin(), path.end()) };
	}
};