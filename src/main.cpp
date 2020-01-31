#include <iostream>

#include <memory>
#include <node.h>

#include "parser.h"

int main(int argc, char ** argv) {

    int32_t tmp = 123213;
    double tmpDouble = 0.1231;
    std::shared_ptr<Node<int32_t>> node(new Node<int32_t>(1, &tmp));
    std::shared_ptr<Node<double>> noded(new Node<double>(1, &tmpDouble));

    std::cout << (*node)[0] << std::endl;

    std::shared_ptr<NodeCompound> comp(new NodeCompound());
    comp->addChild("test", node);
    comp->addChild("dTest", noded);

    std::shared_ptr<Node<double>> test = comp->getNode<double>("dTest");

    std::shared_ptr<NodeCompound> root = parseFile("test.conf");

    std::cout << root << std::endl;

}
