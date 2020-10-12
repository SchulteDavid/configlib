#include <configloading.h>

using namespace config;

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

template <typename T> void defineNode(py::module module, py::class_<NodeBase, std::shared_ptr<NodeBase>> & nodeBase, const char * name) {

  py::class_<Node<T>, std::shared_ptr<Node<T>>>(module, name, nodeBase)
    .def(py::init<size_t, const T * >())
    .def("getElementCount", &Node<T>::getElementCount)
    .def("getElement", &Node<T>::getElement);
  
}

PYBIND11_MODULE(config, m) {

  m.def("parseFile", &config::parseFile, "Parse a file and get a tree-structure");

  py::class_<NodeBase, std::shared_ptr<NodeBase>> nodeBase(m, "NodeBase");
  nodeBase.def(py::init<>())
    .def("getTypeName", &NodeBase::getTypeName)
    .def("getValueString", &NodeBase::getValueString);

  defineNode<int>(m, nodeBase, "NodeInt32");
  defineNode<long>(m, nodeBase, "NodeInt64");
  defineNode<float>(m, nodeBase, "NodeFloat32");
  defineNode<double>(m, nodeBase, "NodeFloat64");
  defineNode<std::shared_ptr<NodeCompound>>(m, nodeBase, "NodeNodeCompArray");
  //defineNode<char>(m, nodeBase, "NodeString");

  py::class_<Node<char>, std::shared_ptr<Node<char>>>(m, "NodeString", nodeBase)
    .def(py::init<size_t, const char * >())
    .def("length", &Node<char>::getElementCount)
    .def("getValue", &Node<char>::getValueString);
  
  py::class_<NodeCompound, std::shared_ptr<NodeCompound>>(m, "NodeCompound", nodeBase)
    .def(py::init<>())
    .def("hasChild", &NodeCompound::hasChild)
    .def("getChild", &NodeCompound::getChild)
    .def("addChild", &NodeCompound::addChild)
    .def("getNodeInt32", &NodeCompound::getNode<int>)
    .def("getNodeInt64", &NodeCompound::getNode<long>)
    .def("getNodeFloat32", &NodeCompound::getNode<float>)
    .def("getNodeFloat64", &NodeCompound::getNode<double>)
    .def("getNodeCompound", &NodeCompound::getNodeCompound)
    .def("getChildMap", &NodeCompound::getChildMap)
    .def("saveAsFile", &NodeCompound::saveAsFile);
  
}
