#include "pyunrealsdk/pch.h"
#include "pyunrealsdk/unreal_bindings/gbxdefptr.h"
#include "unrealsdk/unreal/classes/properties/fgbxdefptrproperty.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/wrappers/wrapped_struct.h"

#ifdef PYUNREALSDK_INTERNAL

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

void register_gbxdefptr(py::module_& mod) {
    py::class_<FGbxDefPtr>(mod, "GbxDefPtr")
        .def(py::init<>(),
             "Creates a default (empty) GbxDefPtr.\n")
        .def(py::init<const FName&, UScriptStruct*, void*>(),
             "Creates a GbxDefPtr.\n"
             "\n"
             "Args:\n"
             "    name: The name of the definition.\n"
             "    ref: The script struct type.\n"
             "    instance_ptr: The pointer to the instance data.",
             "name"_a, "ref"_a, "instance_ptr"_a = nullptr)
        .def_readwrite("name", &FGbxDefPtr::name,
                       "The name of this definition.")
        .def_readwrite("ref", &FGbxDefPtr::ref,
                       "The script struct type this refers to.")
        .def_property_readonly(
            "instance",
            [](const FGbxDefPtr& self) -> std::optional<WrappedStruct> {
                if (self.ref == nullptr || self.instancePtr == nullptr) {
                    return std::nullopt;
                }
                return WrappedStruct{self.ref, self.instancePtr};
            },
            "The instance data as a WrappedStruct, or None if not available.")
        .def("__repr__", [](const FGbxDefPtr& self) {
            std::string name_str = self.name;
                 std::string ref_name =
                     self.ref != nullptr ? self.ref->Name().operator std::string() : "None";
            return std::format("GbxDefPtr(name='{}', ref={}, instance_ptr={:#x})",
                               name_str, ref_name, reinterpret_cast<uintptr_t>(self.instancePtr));
        })
        .def("__bool__", [](const FGbxDefPtr& self) {
            // Consider non-empty if we have a valid name or ref
            return self.ref != nullptr || self.instancePtr != nullptr;
        });
}

}  // namespace pyunrealsdk::unreal

#endif