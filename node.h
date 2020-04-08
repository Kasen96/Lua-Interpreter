#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <iomanip>
#include <list>
#include <vector>
#include <cmath>
#include <string>

using std::string;
using std::cout;
using std::endl;

class Node
{
    public:
        string tag, value;
        std::list<Node> children;

        Node();
        Node(string t, string v);

        //void dump(int);
        void drawDigraph(int, string &);

        Node run();

    private:
        Node getChildNode(int);
        Node print(Node);
        double getArgsNum(Node);
};

#endif