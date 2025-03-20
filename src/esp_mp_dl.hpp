#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "py/obj.h"
#include "py/runtime.h"

#ifdef __cplusplus
}
#endif

#include "dl_image_define.hpp"
#include <memory>

#ifdef __cplusplus
extern "C" {
#endif

extern const mp_obj_type_t mp_face_detector_type;
extern const mp_obj_type_t mp_image_net_type;

#define MP_DEFINE_CONST_FUN_OBJ_0_CXX(obj_name, fun_name) \
    const mp_obj_fun_builtin_fixed_t obj_name = {.base = &mp_type_fun_builtin_0, .fun = {._0 = fun_name }}

#define MP_DEFINE_CONST_FUN_OBJ_1_CXX(obj_name, fun_name) \
    const mp_obj_fun_builtin_fixed_t obj_name = { .base = &mp_type_fun_builtin_1, .fun = {._1 = fun_name }}

#define MP_DEFINE_CONST_FUN_OBJ_2_CXX(obj_name, fun_name) \
    const mp_obj_fun_builtin_fixed_t obj_name = { .base = &mp_type_fun_builtin_2, .fun = {._2 = fun_name }}

#define MP_DEFINE_CONST_FUN_OBJ_3_CXX(obj_name, fun_name) \
    const mp_obj_fun_builtin_fixed_t obj_name = { .base = &mp_type_fun_builtin_3, .fun = {._3 = fun_name }}

#ifdef __cplusplus
}
#endif
