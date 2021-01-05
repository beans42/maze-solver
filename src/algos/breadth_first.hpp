#pragma once
#include "../includes.hpp"

struct breadth_first : solution_interface {
	ret_t solve(const maze_t& maze) {
		auto idx = [&maze](const point_t& p) { return p.y * maze.width + p.x; };

		std::queue<point_t> queue;
		std::vector<point_t> previous(maze.width * maze.height, { -1, -1 });
		std::vector<bool> visited(maze.width * maze.height, false);
		std::vector<unsigned> distances(maze.width * maze.height, UINT_MAX);

		queue.push(maze.start);
		distances[idx(maze.start)] = 0;
		visited[idx(maze.start)] = true;

		auto completed = false;

		while (!queue.empty()) {
			const auto current = queue.front();
			queue.pop();

			if (!maze.grid[idx(current)])
				continue;

			if (current == maze.end) {
				completed = true;
				break;
			}

			for (const auto& v : current.neighbours(maze.width, maze.height)) {
				if (!maze.grid[idx(v)]) continue;
				if (!visited[idx(v)]) {
					queue.push(v);
					distances[idx(v)] = distances[idx(current)] + 1;
					visited[idx(v)] = true;
					previous[idx(v)] = current;
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