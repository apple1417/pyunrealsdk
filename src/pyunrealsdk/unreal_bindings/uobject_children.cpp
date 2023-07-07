#include "pyunrealsdk/unreal_bindings/uobject_children.h"
#include "pyunrealsdk/unreal_bindings/bindings.h"
#include "unrealsdk/unreal/classes/properties/copyable_property.h"
#include "unrealsdk/unreal/classes/properties/uarrayproperty.h"
#include "unrealsdk/unreal/classes/properties/uboolproperty.h"
#include "unrealsdk/unreal/classes/properties/uclassproperty.h"
#include "unrealsdk/unreal/classes/properties/uinterfaceproperty.h"
#include "unrealsdk/unreal/classes/properties/uobjectproperty.h"
#include "unrealsdk/unreal/classes/properties/ustrproperty.h"
#include "unrealsdk/unreal/classes/properties/ustructproperty.h"
#include "unrealsdk/unreal/classes/uclass.h"
#include "unrealsdk/unreal/classes/ufield.h"
#include "unrealsdk/unreal/classes/ufunction.h"
#include "unrealsdk/unreal/classes/uproperty.h"
#include "unrealsdk/unreal/classes/uscriptstruct.h"
#include "unrealsdk/unreal/classes/ustruct.h"

using namespace unrealsdk::unreal;

namespace pyunrealsdk::unreal {

void register_uobject_children(py::module_& mod) {
    PyUEClass<UField, UObject>(mod, "UField", "An unreal field object.")
        .def_readwrite("Next", &UField::Next);

    PyUEClass<UStruct, UField>(mod, "UStruct", "An unreal struct object.")
        .def(
            "_fields",
            [](UStruct* self) {
                auto fields = self->fields();
                // I don't like how calling this with the `fields` directly takes an lvalue
                // reference - would prefer to move an rvalue.
                // Testing, everything still works fine, there's no memory leak, but prefer to
                // manually call this with the iterators anyway
                return py::make_iterator(fields.begin(), fields.end());
            },
            "Iterates over all fields in the struct.\n"
            "\n"
            "Returns:\n"
            "    An iterator over all fields in the struct.")
        .def(
            "_properties",
            [](UStruct* self) {
                auto props = self->properties();
                return py::make_iterator(props.begin(), props.end());
            },
            "Iterates over all properties in the struct.\n"
            "\n"
            "Returns:\n"
            "    An iterator over all properties in the struct.")
        .def("_inherits", &UStruct::inherits,
             "Checks if this structs inherits from another.\n"
             "\n"
             "Also returns true if this struct *is* the given struct.\n"
             "\n"
             "Args:\n"
             "    base_struct: The base struct to check if this inherits from.\n"
             "Returns:\n"
             "    True if this struct is the given struct, or inherits from it.",
             "base_struct"_a)
        .def("_get_struct_size", &UStruct::get_struct_size,
             "Gets the actual size of the described structure, including alignment.\n"
             "\n"
             "Returns:\n"
             "    The size which must be allocated.")
        .def("_find", &UStruct::find,
             "Finds a child field by name.\n"
             "\n"
             "Throws an exception if the child is not found.\n"
             "\n"
             "Args:\n"
             "    name: The name of the child field.\n"
             "Returns:\n"
             "    The found child field.",
             "name"_a)
        .def("_find_prop", &UStruct::find_prop,
             "Finds a child property by name.\n"
             "\n"
             "When known to be a property, this is more efficient than _find.\n"
             "\n"
             "Throws an exception if the child is not found.\n"
             "\n"
             "Args:\n"
             "    name: The name of the child property.\n"
             "Returns:\n"
             "    The found child property.",
             "name"_a)
        .def_readwrite("SuperField", &UStruct::SuperField)
        .def_readwrite("Children", &UStruct::Children)
        .def_readwrite("PropertyLink", &UStruct::PropertyLink);

    PyUEClass<UClass, UStruct>(mod, "UClass", "An unreal class object.")
        .def(
            "_implements",
            [](UClass* self, UClass* iface) { return self->implements(iface, nullptr); },
            "Checks if this class implements a given interface.\n"
            "\n"
            "Args:\n"
            "    iface: The interface to check.\n"
            "Returns:\n"
            "    True if this class implements the interface, false otherwise.")
        .def_readwrite("ClassDefaultObject", &UClass::ClassDefaultObject);

    PyUEClass<UFunction, UStruct>(mod, "UFunction", "An unreal function object.")
        .def("_find_return_param", &UFunction::find_return_param,
             "Finds the return param for this function (if it exists).\n"
             "\n"
             "Returns:\n"
             "    The return param, or None if it doesn't exist.")
        .def_readwrite("FunctionFlags", &UFunction::FunctionFlags)
        .def_readwrite("NumParams", &UFunction::NumParams)
        .def_readwrite("ParamsSize", &UFunction::ParamsSize)
        .def_readwrite("ReturnValueOffset", &UFunction::ReturnValueOffset);

    PyUEClass<UScriptStruct, UStruct>(mod, "UScriptStruct", "An unreal script struct object.")
        .def_readwrite("StructFlags", &UScriptStruct::StructFlags);

    PyUEClass<UProperty, UField>(mod, "UProperty", "The base class of all unreal properties.")
        .def_readwrite("ArrayDim", &UProperty::ArrayDim)
        .def_readwrite("ElementSize", &UProperty::ElementSize)
        .def_readwrite("PropertyFlags", &UProperty::PropertyFlags)
        .def_readwrite("Offset_Internal", &UProperty::Offset_Internal)
        .def_readwrite("PropertyLinkNext", &UProperty::PropertyLinkNext);

    PyUEClass<UInt8Property, UProperty>(mod, "UInt8Property", "An unreal int8 property object.");
    PyUEClass<UInt16Property, UProperty>(mod, "UInt16Property", "An unreal int16 property object.");
    PyUEClass<UIntProperty, UProperty>(mod, "UIntProperty", "An unreal int32 property object.");
    PyUEClass<UInt64Property, UProperty>(mod, "UInt64Property", "An unreal int64 property object.");
    PyUEClass<UByteProperty, UProperty>(mod, "UByteProperty", "An unreal uint8 property object.");
    PyUEClass<UUInt16Property, UProperty>(mod, "UUInt16Property",
                                          "An unreal uint16 property object.");
    PyUEClass<UUInt32Property, UProperty>(mod, "UUInt32Property",
                                          "An unreal uint32 property object.");
    PyUEClass<UUInt64Property, UProperty>(mod, "UUInt64Property",
                                          "An unreal uint64 property object.");
    PyUEClass<UFloatProperty, UProperty>(mod, "UFloatProperty", "An unreal float property object.");
    PyUEClass<UDoubleProperty, UProperty>(mod, "UDoubleProperty",
                                          "An unreal double property object.");
    PyUEClass<UNameProperty, UProperty>(mod, "UNameProperty", "An unreal name property object.");

    PyUEClass<UArrayProperty, UProperty>(mod, "UArrayProperty", "An unreal array property object.")
        .def_property_readonly("Inner", &UArrayProperty::get_inner);

    PyUEClass<UBoolProperty, UProperty>(mod, "UBoolProperty", "An unreal bool property object.")
        .def_property_readonly("FieldMask", &UBoolProperty::get_field_mask);
    PyUEClass<UInterfaceProperty, UProperty>(mod, "UInterfaceProperty",
                                             "An unreal interface property object.")
        .def_property_readonly("InterfaceClass", &UInterfaceProperty::get_interface_class);
    PyUEClass<UObjectProperty, UProperty>(mod, "UObjectProperty",
                                          "An unreal object property object.")
        .def_property_readonly("PropertyClass", &UObjectProperty::get_property_class);
    PyUEClass<UStrProperty, UProperty>(mod, "UStrProperty", "An unreal string property object.");
    PyUEClass<UStructProperty, UProperty>(mod, "UStructProperty",
                                          "An unreal struct property object.")
        .def_property_readonly("Struct", &UStructProperty::get_inner_struct);

    PyUEClass<UClassProperty, UObjectProperty>(mod, "UClassProperty",
                                               "An unreal class property object.")
        .def_property_readonly("MetaClass", &UClassProperty::get_meta_class);
}

}  // namespace pyunrealsdk::unreal
