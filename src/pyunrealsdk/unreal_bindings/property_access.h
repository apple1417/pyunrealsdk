#ifndef PYUNREALSDK_UNREAL_BINDINGS_PROPERTY_ACCESS_H
#define PYUNREALSDK_UNREAL_BINDINGS_PROPERTY_ACCESS_H

#include "pyunrealsdk/pch.h"

namespace unrealsdk::unreal {

class UObject;
class UField;
class UStruct;
class UProperty;

}  // namespace unrealsdk::unreal

namespace pyunrealsdk::unreal {

/**
 * @brief Registers the helpers functions for property accesses.
 *
 * @param mod The module to register within.
 */
void register_property_helpers(py::module_& mod);

/**
 * @brief Gets a field off of an object based on the key given to getattr/setattr.
 * @note Allows both strings and direct field references.
 *
 * @param key The python key.
 * @param type The type of the unreal object this access is reading off of.
 * @return The field. Invalid keys throw, so will never be null.
 */
unrealsdk::unreal::UField* get_field_from_py_key(const py::object& key,
                                                 const unrealsdk::unreal::UStruct* type);

/**
 * @brief Implements `__dir__`.
 *
 * @param self The base object - used to get the non-dynamic fields.
 * @param type The type of the object.
 * @return A list of all attributes on the object.
 */
std::vector<std::string> py_dir(const py::object& self, const unrealsdk::unreal::UStruct* type);

/**
 * @brief Implements `__getattr__`.
 *
 * @param field The field to get.
 * @param base_addr The base address of the object.
 * @param func_obj The object to bind functions to. If nullptr, does not allow getting functions.
 * @return The retrieved value.
 */
py::object py_getattr(unrealsdk::unreal::UField* field,
                      uintptr_t base_addr,
                      unrealsdk::unreal::UObject* func_obj = nullptr);

/**
 * @brief Implements `__setattr__`.
 *
 * @param field The field to get.
 * @param base_addr The base address of the object.
 * @param value The value to set.
 */
void py_setattr(unrealsdk::unreal::UField* field, uintptr_t base_addr, const py::object& value);

}  // namespace pyunrealsdk::unreal

#endif /* PYUNREALSDK_UNREAL_BINDINGS_PROPERTY_ACCESS_H */
