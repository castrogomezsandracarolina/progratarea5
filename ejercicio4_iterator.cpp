#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <stdexcept>
#include <unordered_set>
#include <vector>

class Node {
private:
    int id_;
    std::vector<int> neighbors_;
    friend class Graph;
public:
    explicit Node(int id) : id_(id) {}
};

class Graph {
private:
    std::map<int, Node> nodes_;
    void addNode(int id) {
        nodes_.try_emplace(id, id);
    }
public:
    void addEdge(int from, int to) {
        addNode(from);
        addNode(to);
        auto& neighbors = nodes_.at(from).neighbors_;
        if (std::find(neighbors.begin(), neighbors.end(), to) == neighbors.end()) {
            neighbors.push_back(to);
        }
    }
    bool hasNode(int id) const {
        return nodes_.find(id) != nodes_.end();
    }

    void forEachNeighbor(int id, const std::function<void(int)>& visitor) const {
        const auto it = nodes_.find(id);
        if (it == nodes_.end()) {
            throw std::out_of_range("Nodo no encontrado");
        }
        for (int neighbor : it->second.neighbors_) {
            visitor(neighbor);
        }
    }
};

class GraphIterator {
public:
    virtual ~GraphIterator() = default;
    virtual bool hasNext() const = 0;
    virtual int next() = 0;
};

class BFSIterator : public GraphIterator {
private:
    const Graph& graph_;
    std::queue<int> pending_;
    std::unordered_set<int> visited_;
public:
    BFSIterator(const Graph& graph, int start) : graph_(graph) {
        if (!graph_.hasNode(start)) {
            throw std::invalid_argument("Nodo inicial inexistente");
        }
        pending_.push(start);
        visited_.insert(start);
    }
    bool hasNext() const override {
        return !pending_.empty();
    }
    int next() override {
        if (!hasNext()) {
            throw std::out_of_range("No quedan nodos por visitar");
        }
        int current = pending_.front();
        pending_.pop();
        graph_.forEachNeighbor(current, [&](int neighbor) {
            if (visited_.insert(neighbor).second) {
                pending_.push(neighbor);
            }
        });
        return current;
    }
};

class DFSIterator : public GraphIterator {
private:
    const Graph& graph_;
    std::vector<int> pending_;
    std::unordered_set<int> visited_;
public:
    DFSIterator(const Graph& graph, int start) : graph_(graph) {
        if (!graph_.hasNode(start)) {
            throw std::invalid_argument("Nodo inicial inexistente");
        }
        pending_.push_back(start);
    }
    bool hasNext() const override {
        for (int id : pending_) {
            if (visited_.count(id) == 0) return true;
        }
        return false;
    }
    int next() override {
        while (!pending_.empty()) {
            int current = pending_.back();
            pending_.pop_back();
            if (!visited_.insert(current).second) continue;
            std::vector<int> neighbors;
            graph_.forEachNeighbor(current, [&](int id) {
                neighbors.push_back(id);
            });
            for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it) {
                if (visited_.count(*it) == 0) pending_.push_back(*it);
            }
            return current;
        }
        throw std::out_of_range("No quedan nodos por visitar");
    }
};

int main() {
    Graph g;
    g.addEdge(1, 2);
    g.addEdge(1, 3);
    g.addEdge(2, 4);
    g.addEdge(4, 1);
    std::unique_ptr<GraphIterator> it = std::make_unique<BFSIterator>(g, 1);
    std::cout << "BFS: ";
    while (it->hasNext()) {
        std::cout << it->next() << ' ';
    }
    std::cout << '\n';
    it = std::make_unique<DFSIterator>(g, 1);
    std::cout << "DFS: ";
    while (it->hasNext()) {
        std::cout << it->next() << ' ';
    }
    std::cout << '\n';
    return 0;
}
