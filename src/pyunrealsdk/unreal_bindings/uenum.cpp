#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/uenum.h"
#include "pyunrealsdk/static_py_object.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "unrealsdk/unreal/classes/uenum.h"
#include "unrealsdk/unreal/classes/ufield.h"

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

PYUNREALSDK_CAPI(PyObject*, enum_as_py_enum, const UEnum* enum_obj);

#ifdef PYUNREALSDK_INTERNAL

void register_uenum(py::module_& mod) {
    PyUEClass<UEnum, UField>(mod, "UEnum")
        .def("_as_py", &enum_as_py_enum,
             "Generates a compatible IntFlag enum.\n"
             "\n"
             "Returns:\n"
             "    An IntFlag enum compatible with this enum.");
}

py::object enum_as_py_enum(const UEnum* enum_obj) {
    const py::gil_scoped_acquire gil{};

    // Use IntFlag, as it natively supports unknown values
    PYBIND11_CONSTINIT static py::gil_safe_call_once_and_store<py::object> storage;
    auto& intflag = storage
                        .call_once_and_store_result(
                            []() { return py::module_::import("enum").attr("IntFlag"); })
                        .get_stored();

    static std::unordered_map<const UEnum*, StaticPyObject> enum_cache{};

    if (!enum_cache.contains(enum_obj)) {
        std::unordered_map<FName, uint64_t> enum_names{};

#ifdef UE4
        // UE4 enums include the enum name and a namespace separator before the name - strip them

        for (const auto& [key, value] : enum_obj->get_names()) {
            const std::string str_key{key};

            auto after_colons = str_key.find_first_not_of(':', str_key.find_first_of(':'));
            enum_names.emplace(
                after_colons == std::string::npos ? str_key : str_key.substr(after_colons), value);
        }
#else
        // UE3 enums are just the name, so we can use the dict directly
        enum_names = enum_obj->get_names();
#endif

        auto py_enum = intflag(enum_obj->Name(), enum_names);
        py_enum.attr("_unreal") = enum_obj;

        enum_cache.emplace(enum_obj, py_enum);
    }

    return enum_cache[enum_obj];
}

PYUNREALSDK_CAPI(PyObject*, enum_as_py_enum, const UEnum* enum_obj) {
    const py::gil_scoped_acquire gil{};

    auto obj = enum_as_py_enum(enum_obj);
    // Add an extra ref, so the python object sticks around when we destroy the py::object
    obj.inc_ref();
    return obj.ptr();
}
#else
py::object enum_as_py_enum(const UEnum* enum_obj) {
    const py::gil_scoped_acquire gil{};

    // Steal the reference which we incremented on the other side of this call
    return py::reinterpret_steal<py::object>(PYUNREALSDK_MANGLE(enum_as_py_enum)(enum_obj));
}
#endif

}  // namespace pyunrealsdk::unreal
