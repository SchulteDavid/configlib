#include <iostream>

#include <memory>
#include <node.h>
#include <configloading.h>
#include <fstream>

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

    std::shared_ptr<NodeCompound> root = config::parseFile("test.conf");

    std::cout << root->getNode<int32_t>("testInt")->getElement(0) << std::endl;
    std::cout << root->getNode<double>("doubleVal")->getElement(2) << std::endl;

    auto testCompArray = root->getNode<std::shared_ptr<NodeCompound>>("testCompArray");
    std::cout << testCompArray->getElement(0)->getNode<int32_t>("test")->getElement(0) << std::endl;

    std::ofstream out("out.conf");

    root->saveAsFile(out);

    out.close();

}
