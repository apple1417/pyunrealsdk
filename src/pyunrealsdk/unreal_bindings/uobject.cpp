#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/uobject.h"
#include "pyunrealsdk/static_py_object.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "pyunrealsdk/unreal_bindings/property_access.h"
#include "unrealsdk/format.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/uobject.h"
#include "unrealsdk/unreal/structs/fname.h"
#include "unrealsdk/unrealsdk.h"
#include "unrealsdk/utils.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

namespace {

// This needs to be a function to avoid an unreachable code warning in `pybind11/detail/init.h` in
// `no_nullptr` when compiling with MSVC.
// I assume this is something to do with inlining optimizations - since we don't actually return, it
// can assume it's nullptr, somehow ignoring the throw here, but not the one later?
// Not thrilled with this as a solution, but it works for now

// __init__
UObject* uobject_init(const py::args& /* args */, const py::kwargs& /* kwargs */) {
    throw py::type_error("Cannot create new instances of unreal objects.");
}

}  // namespace

void register_uobject(py::module_& mod) {
    PyUEClass<UObject>(
        mod, "UObject",
        "The base class of all unreal objects.\n"
        "\n"
        "Most objects you interact with will be this type in python, even if their unreal\n"
        "class is something different.")
        .def("__new__",
             [](const py::args&, const py::kwargs&) {
                 throw py::type_error("Cannot create new instances of unreal objects.");
             })
        .def(py::init(&uobject_init))
        .def(
            "__repr__",
            [](UObject* self) {
                return unrealsdk::fmt::format("{}'{}'", self->Class->Name,
                                              unrealsdk::utils::narrow(self->get_path_name()));
            },
            "Gets this object's full name.\n"
            "\n"
            "Returns:\n"
            "    This object's name.")
        .def("_path_name", &UObject::get_path_name,
             "Gets this object's path name, excluding the class.\n"
             "\n"
             "Returns:\n"
             "    This object's name.")
        .def(
            "__dir__",
            [](const py::object& self) { return py_dir(self, py::cast<UObject*>(self)->Class); },
            "Gets the attributes which exist on this object.\n"
            "\n"
            "Includes both python attributes and unreal fields. This can be changed to only\n"
            "python attributes by calling dir_includes_unreal.\n"
            "\n"
            "Returns:\n"
            "    A list of attributes which exist on this object.")
        .def(
            "__getattr__",
            [](UObject* self, const FName& name) {
                return py_getattr(py_find_field(name, self->Class),
                                  reinterpret_cast<uintptr_t>(self), nullptr, self);
            },
            "Reads an unreal field off of the object.\n"
            "\n"
            "Automatically looks up the relevant UField.\n"
            "\n"
            "Args:\n"
            "    name: The name of the field to get.\n"
            "Returns:\n"
            "    The field's value.",
            "name"_a)
        .def(
            "_get_field",
            [](UObject* self, UField* field) {
                if (field == nullptr) {
                    throw py::attribute_error("cannot access null attribute");
                }
                return py_getattr(field, reinterpret_cast<uintptr_t>(self), nullptr, self);
            },
            "Reads an unreal field off of the object.\n"
            "\n"
            "In performance critical situations, rather than use __getattr__, you can look up\n"
            "the UField beforehand (via obj.Class._find()), then pass it directly to this\n"
            "function. This does not get validated, passing a field which doesn't exist on\n"
            "the object is undefined behaviour.\n"
            "\n"
            "Args:\n"
            "    field: The field to get.\n"
            "Returns:\n"
            "    The field's value.",
            "field"_a)
        .def(
            "__setattr__",
            [](UObject* self, const py::str& name, const py::object& value) {
                // See if the standard setattr would work first, in case we're being called on an
                // existing field. Getattr is only called on failure, but setattr is always called.
                PYBIND11_CONSTINIT static py::gil_safe_call_once_and_store<py::object> storage;
                auto& setattr = storage
                                    .call_once_and_store_result([]() {
                                        return py::module::import("builtins")
                                            .attr("object")
                                            .attr("__setattr__");
                                    })
                                    .get_stored();

                try {
                    setattr(self, name, value);
                    return;
                } catch (py::error_already_set& e) {
                    if (!e.matches(PyExc_AttributeError)) {
                        throw;
                    }
                }

                py_setattr_direct(py_find_field(py::cast<FName>(name), self->Class),
                                  reinterpret_cast<uintptr_t>(self), value);
            },
            "Writes a value to an unreal field on the object.\n"
            "\n"
            "Automatically looks up the relevant UField.\n"
            "\n"
            "Args:\n"
            "    name: The name of the field to set.\n"
            "    value: The value to write.",
            "name"_a, "value"_a)
        .def(
            "_set_field",
            [](UObject* self, UField* field, const py::object& value) {
                if (field == nullptr) {
                    throw py::attribute_error("cannot access null attribute");
                }
                py_setattr_direct(field, reinterpret_cast<uintptr_t>(self), value);
            },
            "Writes a value to an unreal field on the object.\n"
            "\n"
            "In performance critical situations, rather than use __setattr__, you can look up\n"
            "the UField beforehand (via obj.Class._find()), then pass it directly to this\n"
            "function. This does not get validated, passing a field which doesn't exist on\n"
            "the object is undefined behaviour.\n"
            "\n"
            "Args:\n"
            "    field: The field to set.\n"
            "    value: The value to write.",
            "field"_a, "value"_a)
        .def(
            "_get_address", [](UObject* self) { return reinterpret_cast<uintptr_t>(self); },
            "Gets the address of this object, for debugging.\n"
            "\n"
            "Returns:\n"
            "    This object's address.")
        .def_readwrite("ObjectFlags", &UObject::ObjectFlags)
        .def_readwrite("InternalIndex", &UObject::InternalIndex)
        .def_readwrite("Class", &UObject::Class)
        .def_readwrite("Name", &UObject::Name)
        .def_readwrite("Outer", &UObject::Outer);
}

}  // namespace pyunrealsdk::unreal

#endif
