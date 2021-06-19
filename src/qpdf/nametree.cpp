/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (C) 2019, James R. Barlow (https://github.com/jbarlow83/)
 */

#include <qpdf/Constants.h>
#include <qpdf/Types.h>
#include <qpdf/DLL.h>
#include <qpdf/QPDFExc.hh>
#include <qpdf/PointerHolder.hh>
#include <qpdf/QPDFNameTreeObjectHelper.hh>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "pikepdf.h"

class NameTreeHolder {
public:
    NameTreeHolder(QPDFObjectHandle oh, QPDF &q, bool auto_repair = true)
        : ntoh(oh, q, auto_repair)
    {
    }

    QPDFObjectHandle getObjectHandle() { return this->ntoh.getObjectHandle(); }

    bool hasName(const std::string &utf8) { return this->ntoh.hasName(utf8); }

    bool findObject(const std::string &utf8, QPDFObjectHandle &oh)
    {
        return this->ntoh.findObject(utf8, oh);
    }

    std::map<std::string, QPDFObjectHandle> getAsMap() const
    {
        return this->ntoh.getAsMap();
    }

    void insert(std::string const &key, QPDFObjectHandle value)
    {
        (void)this->ntoh.insert(key, value);
    }

    void remove(std::string const &key)
    {
        bool result = this->ntoh.remove(key);
        if (!result)
            throw py::key_error(key);
    }

private:
    QPDFNameTreeObjectHelper ntoh;
};

void init_nametree(py::module_ &m)
{
    py::class_<NameTreeHolder, std::shared_ptr<NameTreeHolder>>(m, "NameTree")
        .def(py::init<QPDFObjectHandle, QPDF &, bool>(),
            py::arg("oh"),
            py::arg("q"),
            py::kw_only(),
            py::arg("auto_repair") = true)
        .def_property_readonly(
            "obj",
            [](NameTreeHolder &nt) { return nt.getObjectHandle(); },
            "Returns the underlying root object for this name tree.")
        .def(
            "__contains__",
            [](NameTreeHolder &nt, std::string const &name) {
                return nt.hasName(name);
            },
            R"~~~(
        Returns True if the name tree contains the specified name.

        Args:
            name (str or bytes): The name to search for in the name tree.
                This is not a PDF /Name object, but an arbitrary key.
                If name is a *str*, we search the name tree for the UTF-8
                encoded form of name. If *bytes*, we search for a key
                equal to those bytes.
        )~~~")
        .def("__getitem__",
            [](NameTreeHolder &nt, std::string const &name) {
                QPDFObjectHandle oh;
                if (nt.findObject(name, oh))
                    return py::cast(oh);
                else
                    throw py::key_error(name);
            })
        .def("__setitem__",
            [](NameTreeHolder &nt, std::string const &name, QPDFObjectHandle oh) {
                nt.insert(name, oh);
            })
        .def("__delitem__",
            [](NameTreeHolder &nt, std::string const &name) { nt.remove(name); })
        .def("_as_map", [](NameTreeHolder &nt) { return nt.getAsMap(); });
}
