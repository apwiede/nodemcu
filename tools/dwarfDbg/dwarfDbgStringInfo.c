/*
* Copyright (c) 2017, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
* All rights reserved.
*
* License: BSD/MIT
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its
* contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/

/* 
 * File:   dwarfDbgStringInfo.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on February 05, 2017
 */

#include "dwarfDbgInt.h"

static id2Str_t DW_TAG_stringInfos [] = {
  { DW_TAG_array_type,                     "DW_TAG_array_type" },
  { DW_TAG_class_type,                     "DW_TAG_class_type" },
  { DW_TAG_entry_point,                    "DW_TAG_entry_point" },
  { DW_TAG_enumeration_type,               "DW_TAG_enumeration_type" },
  { DW_TAG_formal_parameter,               "DW_TAG_formal_parameter" },
  { DW_TAG_imported_declaration,           "DW_TAG_imported_declaration" },
  { DW_TAG_label,                          "DW_TAG_label" },
  { DW_TAG_lexical_block,                  "DW_TAG_lexical_block" },
  { DW_TAG_member,                         "DW_TAG_member" },
  { DW_TAG_pointer_type,                   "DW_TAG_pointer_type" },
  { DW_TAG_reference_type,                 "DW_TAG_reference_type" },
  { DW_TAG_compile_unit,                   "DW_TAG_compile_unit" },
  { DW_TAG_string_type,                    "DW_TAG_string_type" },
  { DW_TAG_structure_type,                 "DW_TAG_structure_type" },
  { DW_TAG_subroutine_type,                "DW_TAG_subroutine_type" },
  { DW_TAG_typedef,                        "DW_TAG_typedef" },
  { DW_TAG_union_type,                     "DW_TAG_union_type" },
  { DW_TAG_unspecified_parameters,         "DW_TAG_unspecified_parameters" },
  { DW_TAG_variant,                        "DW_TAG_variant" },
  { DW_TAG_common_block,                   "DW_TAG_common_block" },
  { DW_TAG_common_inclusion,               "DW_TAG_common_inclusion" },
  { DW_TAG_inheritance,                    "DW_TAG_inheritance" },
  { DW_TAG_inlined_subroutine,             "DW_TAG_inlined_subroutine" },
  { DW_TAG_module,                         "DW_TAG_module" },
  { DW_TAG_ptr_to_member_type,             "DW_TAG_ptr_to_member_type" },
  { DW_TAG_set_type,                       "DW_TAG_set_type" },
  { DW_TAG_subrange_type,                  "DW_TAG_subrange_type" },
  { DW_TAG_with_stmt,                      "DW_TAG_with_stmt" },
  { DW_TAG_access_declaration,             "DW_TAG_access_declaration" },
  { DW_TAG_base_type,                      "DW_TAG_base_type" },
  { DW_TAG_catch_block,                    "DW_TAG_catch_block" },
  { DW_TAG_const_type,                     "DW_TAG_const_type" },
  { DW_TAG_constant,                       "DW_TAG_constant" },
  { DW_TAG_enumerator,                     "DW_TAG_enumerator" },
  { DW_TAG_file_type,                      "DW_TAG_file_type" },
  { DW_TAG_friend,                         "DW_TAG_friend" },
  { DW_TAG_namelist,                       "DW_TAG_namelist" },
  { DW_TAG_namelist_item,                  "DW_TAG_namelist_item" },
  { DW_TAG_namelist_items,                 "DW_TAG_namelist_items" },
  { DW_TAG_packed_type,                    "DW_TAG_packed_type" },
  { DW_TAG_subprogram,                     "DW_TAG_subprogram" },
  { DW_TAG_template_type_parameter,        "DW_TAG_template_type_parameter" },
  { DW_TAG_template_type_param,            "DW_TAG_template_type_param" },
  { DW_TAG_template_value_parameter,       "DW_TAG_template_value_parameter" },
  { DW_TAG_template_value_param,           "DW_TAG_template_value_param" },
  { DW_TAG_thrown_type,                    "DW_TAG_thrown_type" },
  { DW_TAG_try_block,                      "DW_TAG_try_block" },
  { DW_TAG_variant_part,                   "DW_TAG_variant_part" },
  { DW_TAG_variable,                       "DW_TAG_variable" },
  { DW_TAG_volatile_type,                  "DW_TAG_volatile_type" },
  { DW_TAG_dwarf_procedure,                "DW_TAG_dwarf_procedure" },
  { DW_TAG_restrict_type,                  "DW_TAG_restrict_type" },
  { DW_TAG_interface_type,                 "DW_TAG_interface_type" },
  { DW_TAG_namespace,                      "DW_TAG_namespace" },
  { DW_TAG_imported_module,                "DW_TAG_imported_module" },
  { DW_TAG_unspecified_type,               "DW_TAG_unspecified_type" },
  { DW_TAG_partial_unit,                   "DW_TAG_partial_unit" },
  { DW_TAG_imported_unit,                  "DW_TAG_imported_unit" },
  { DW_TAG_mutable_type,                   "DW_TAG_mutable_type" },
  { DW_TAG_condition,                      "DW_TAG_condition" },
  { DW_TAG_shared_type,                    "DW_TAG_shared_type" },
  { DW_TAG_type_unit,                      "DW_TAG_type_unit" },
  { DW_TAG_rvalue_reference_type,          "DW_TAG_rvalue_reference_type" },
  { DW_TAG_template_alias,                 "DW_TAG_template_alias" },
  { DW_TAG_coarray_type,                   "DW_TAG_coarray_type" },
  { DW_TAG_generic_subrange,               "DW_TAG_generic_subrange" },
  { DW_TAG_dynamic_type,                   "DW_TAG_dynamic_type" },
  { DW_TAG_atomic_type,                    "DW_TAG_atomic_type" },
  { DW_TAG_call_site,                      "DW_TAG_call_site" },
  { DW_TAG_call_site_parameter,            "DW_TAG_call_site_parameter" },
  { DW_TAG_skeleton_unit,                  "DW_TAG_skeleton_unit" },
  { DW_TAG_immutable_type,                 "DW_TAG_immutable_type" },
  { DW_TAG_lo_user,                        "DW_TAG_lo_user" },
  { DW_TAG_MIPS_loop,                      "DW_TAG_MIPS_loop" },
  { DW_TAG_HP_array_descriptor,            "DW_TAG_HP_array_descriptor" },
  { DW_TAG_format_label,                   "DW_TAG_format_label" },
  { DW_TAG_function_template,              "DW_TAG_function_template" },
  { DW_TAG_class_template,                 "DW_TAG_class_template" },
  { DW_TAG_GNU_BINCL,                      "DW_TAG_GNU_BINCL" },
  { DW_TAG_GNU_EINCL,                      "DW_TAG_GNU_EINCL" },
  { DW_TAG_GNU_template_template_parameter,"DW_TAG_GNU_template_template_parameter" },
  { DW_TAG_GNU_template_template_param,    "DW_TAG_GNU_template_template_param" },
  { DW_TAG_GNU_template_parameter_pack,    "DW_TAG_GNU_template_parameter_pack" },
  { DW_TAG_GNU_formal_parameter_pack,      "DW_TAG_GNU_formal_parameter_pack" },
  { DW_TAG_GNU_call_site,                  "DW_TAG_GNU_call_site" },
  { DW_TAG_GNU_call_site_parameter,        "DW_TAG_GNU_call_site_parameter" },
  { DW_TAG_ALTIUM_circ_type,               "DW_TAG_ALTIUM_circ_type" },
  { DW_TAG_ALTIUM_mwa_circ_type,           "DW_TAG_ALTIUM_mwa_circ_type" },
  { DW_TAG_ALTIUM_rev_carry_type,          "DW_TAG_ALTIUM_rev_carry_type" },
  { DW_TAG_ALTIUM_rom,                     "DW_TAG_ALTIUM_rom" },
  { DW_TAG_upc_shared_type,                "DW_TAG_upc_shared_type" },
  { DW_TAG_upc_strict_type,                "DW_TAG_upc_strict_type" },
  { DW_TAG_upc_relaxed_type,               "DW_TAG_upc_relaxed_type" },
  { DW_TAG_PGI_kanji_type,                 "DW_TAG_PGI_kanji_type" },
  { DW_TAG_PGI_interface_block,            "DW_TAG_PGI_interface_block" },
  { DW_TAG_SUN_function_template,          "DW_TAG_SUN_function_template" },
  { DW_TAG_SUN_class_template,             "DW_TAG_SUN_class_template" },
  { DW_TAG_SUN_struct_template,            "DW_TAG_SUN_struct_template" },
  { DW_TAG_SUN_union_template,             "DW_TAG_SUN_union_template" },
  { DW_TAG_SUN_indirect_inheritance,       "DW_TAG_SUN_indirect_inheritance" },
  { DW_TAG_SUN_codeflags,                  "DW_TAG_SUN_codeflags" },
  { DW_TAG_SUN_memop_info,                 "DW_TAG_SUN_memop_info" },
  { DW_TAG_SUN_omp_child_func,             "DW_TAG_SUN_omp_child_func" },
  { DW_TAG_SUN_rtti_descriptor,            "DW_TAG_SUN_rtti_descriptor" },
  { DW_TAG_SUN_dtor_info,                  "DW_TAG_SUN_dtor_info" },
  { DW_TAG_SUN_dtor,                       "DW_TAG_SUN_dtor" },
  { DW_TAG_SUN_f90_interface,              "DW_TAG_SUN_f90_interface" },
  { DW_TAG_SUN_fortran_vax_structure,      "DW_TAG_SUN_fortran_vax_structure" },
  { DW_TAG_SUN_hi,                         "DW_TAG_SUN_hi" },
  { DW_TAG_hi_user,                        "DW_TAG_hi_user" },
  { 0, NULL },
};

static id2Str_t DW_FORM_stringInfos [] = {
  { DW_FORM_addr,           "DW_FORM_addr" },
  { DW_FORM_block2,         "DW_FORM_block2" },
  { DW_FORM_block4,         "DW_FORM_block4" },
  { DW_FORM_data2,          "DW_FORM_data2" },
  { DW_FORM_data4,          "DW_FORM_data4" },
  { DW_FORM_data8,          "DW_FORM_data8" },
  { DW_FORM_string,         "DW_FORM_string" },
  { DW_FORM_block,          "DW_FORM_block" },
  { DW_FORM_block1,         "DW_FORM_block1" },
  { DW_FORM_data1,          "DW_FORM_data1" },
  { DW_FORM_flag,           "DW_FORM_flag" },
  { DW_FORM_sdata,          "DW_FORM_sdata" },
  { DW_FORM_strp,           "DW_FORM_strp" },
  { DW_FORM_udata,          "DW_FORM_udata" },
  { DW_FORM_ref_addr,       "DW_FORM_ref_addr" },
  { DW_FORM_ref1,           "DW_FORM_ref1" },
  { DW_FORM_ref2,           "DW_FORM_ref2" },
  { DW_FORM_ref4,           "DW_FORM_ref4" },
  { DW_FORM_ref8,           "DW_FORM_ref8" },
  { DW_FORM_ref_udata,      "DW_FORM_ref_udata" },
  { DW_FORM_indirect,       "DW_FORM_indirect" },
  { DW_FORM_sec_offset,     "DW_FORM_sec_offset" },
  { DW_FORM_exprloc,        "DW_FORM_exprloc" },
  { DW_FORM_flag_present,   "DW_FORM_flag_present" },
  { DW_FORM_strx,           "DW_FORM_strx" },
  { DW_FORM_addrx,          "DW_FORM_addrx" },
  { DW_FORM_ref_sup4,       "DW_FORM_ref_sup4" },
  { DW_FORM_strp_sup,       "DW_FORM_strp_sup" },
  { DW_FORM_data16,         "DW_FORM_data16" },
  { DW_FORM_line_strp,      "DW_FORM_line_strp" },
  { DW_FORM_ref_sig8,       "DW_FORM_ref_sig8" },
  { DW_FORM_implicit_const, "DW_FORM_implicit_const" },
  { DW_FORM_loclistx,       "DW_FORM_loclistx" },
  { DW_FORM_rnglistx,       "DW_FORM_rnglistx" },
  { DW_FORM_ref_sup8,       "DW_FORM_ref_sup8" },
  { DW_FORM_GNU_addr_index, "DW_FORM_GNU_addr_index" },
  { DW_FORM_GNU_str_index,  "DW_FORM_GNU_str_index" },
  { DW_FORM_GNU_ref_alt,    "DW_FORM_GNU_ref_alt" },
  { DW_FORM_GNU_strp_alt,   "DW_FORM_GNU_strp_alt" },
  { 0, NULL },
};

static id2Str_t DW_AT_stringInfos [] = {
  { DW_AT_sibling,                        "DW_AT_sibling" },
  { DW_AT_location,                       "DW_AT_location" },
  { DW_AT_name,                           "DW_AT_name" },
  { DW_AT_ordering,                       "DW_AT_ordering" },
  { DW_AT_subscr_data,                    "DW_AT_subscr_data" },
  { DW_AT_byte_size,                      "DW_AT_byte_size" },
  { DW_AT_bit_offset,                     "DW_AT_bit_offset" },
  { DW_AT_bit_size,                       "DW_AT_bit_size" },
  { DW_AT_element_list,                   "DW_AT_element_list" },
  { DW_AT_stmt_list,                      "DW_AT_stmt_list" },
  { DW_AT_low_pc,                         "DW_AT_low_pc" },
  { DW_AT_high_pc,                        "DW_AT_high_pc" },
  { DW_AT_language,                       "DW_AT_language" },
  { DW_AT_member,                         "DW_AT_member" },
  { DW_AT_discr,                          "DW_AT_discr" },
  { DW_AT_discr_value,                    "DW_AT_discr_value" },
  { DW_AT_visibility,                     "DW_AT_visibility" },
  { DW_AT_import,                         "DW_AT_import" },
  { DW_AT_string_length,                  "DW_AT_string_length" },
  { DW_AT_common_reference,               "DW_AT_common_reference" },
  { DW_AT_comp_dir,                       "DW_AT_comp_dir" },
  { DW_AT_const_value,                    "DW_AT_const_value" },
  { DW_AT_containing_type,                "DW_AT_containing_type" },
  { DW_AT_default_value,                  "DW_AT_default_value" },
  { DW_AT_inline,                         "DW_AT_inline" },
  { DW_AT_is_optional,                    "DW_AT_is_optional" },
  { DW_AT_lower_bound,                    "DW_AT_lower_bound" },
  { DW_AT_producer,                       "DW_AT_producer" },
  { DW_AT_prototyped,                     "DW_AT_prototyped" },
  { DW_AT_return_addr,                    "DW_AT_return_addr" },
  { DW_AT_start_scope,                    "DW_AT_start_scope" },
  { DW_AT_bit_stride,                     "DW_AT_bit_stride" },
  { DW_AT_stride_size,                    "DW_AT_stride_size" },
  { DW_AT_upper_bound,                    "DW_AT_upper_bound" },
  { DW_AT_abstract_origin,                "DW_AT_abstract_origin" },
  { DW_AT_accessibility,                  "DW_AT_accessibility" },
  { DW_AT_address_class,                  "DW_AT_address_class" },
  { DW_AT_artificial,                     "DW_AT_artificial" },
  { DW_AT_base_types,                     "DW_AT_base_types" },
  { DW_AT_calling_convention,             "DW_AT_calling_convention" },
  { DW_AT_count,                          "DW_AT_count" },
  { DW_AT_data_member_location,           "DW_AT_data_member_location" },
  { DW_AT_decl_column,                    "DW_AT_decl_column" },
  { DW_AT_decl_file,                      "DW_AT_decl_file" },
  { DW_AT_decl_line,                      "DW_AT_decl_line" },
  { DW_AT_declaration,                    "DW_AT_declaration" },
  { DW_AT_discr_list,                     "DW_AT_discr_list" },
  { DW_AT_encoding,                       "DW_AT_encoding" },
  { DW_AT_external,                       "DW_AT_external" },
  { DW_AT_frame_base,                     "DW_AT_frame_base" },
  { DW_AT_friend,                         "DW_AT_friend" },
  { DW_AT_identifier_case,                "DW_AT_identifier_case" },
  { DW_AT_macro_info,                     "DW_AT_macro_info" },
  { DW_AT_namelist_item,                  "DW_AT_namelist_item" },
  { DW_AT_priority,                       "DW_AT_priority" },
  { DW_AT_segment,                        "DW_AT_segment" },
  { DW_AT_specification,                  "DW_AT_specification" },
  { DW_AT_static_link,                    "DW_AT_static_link" },
  { DW_AT_type,                           "DW_AT_type" },
  { DW_AT_use_location,                   "DW_AT_use_location" },
  { DW_AT_variable_parameter,             "DW_AT_variable_parameter" },
  { DW_AT_virtuality,                     "DW_AT_virtuality" },
  { DW_AT_vtable_elem_location,           "DW_AT_vtable_elem_location" },
  { DW_AT_allocated,                      "DW_AT_allocated" },
  { DW_AT_associated,                     "DW_AT_associated" },
  { DW_AT_data_location,                  "DW_AT_data_location" },
  { DW_AT_byte_stride,                    "DW_AT_byte_stride" },
  { DW_AT_stride,                         "DW_AT_stride" },
  { DW_AT_entry_pc,                       "DW_AT_entry_pc" },
  { DW_AT_use_UTF8,                       "DW_AT_use_UTF8" },
  { DW_AT_extension,                      "DW_AT_extension" },
  { DW_AT_ranges,                         "DW_AT_ranges" },
  { DW_AT_trampoline,                     "DW_AT_trampoline" },
  { DW_AT_call_column,                    "DW_AT_call_column" },
  { DW_AT_call_file,                      "DW_AT_call_file" },
  { DW_AT_call_line,                      "DW_AT_call_line" },
  { DW_AT_description,                    "DW_AT_description" },
  { DW_AT_binary_scale,                   "DW_AT_binary_scale" },
  { DW_AT_decimal_scale,                  "DW_AT_decimal_scale" },
  { DW_AT_small,                          "DW_AT_small" },
  { DW_AT_decimal_sign,                   "DW_AT_decimal_sign" },
  { DW_AT_digit_count,                    "DW_AT_digit_count" },
  { DW_AT_picture_string,                 "DW_AT_picture_string" },
  { DW_AT_mutable,                        "DW_AT_mutable" },
  { DW_AT_threads_scaled,                 "DW_AT_threads_scaled" },
  { DW_AT_explicit,                       "DW_AT_explicit" },
  { DW_AT_object_pointer,                 "DW_AT_object_pointer" },
  { DW_AT_endianity,                      "DW_AT_endianity" },
  { DW_AT_elemental,                      "DW_AT_elemental" },
  { DW_AT_pure,                           "DW_AT_pure" },
  { DW_AT_recursive,                      "DW_AT_recursive" },
  { DW_AT_signature,                      "DW_AT_signature" },
  { DW_AT_main_subprogram,                "DW_AT_main_subprogram" },
  { DW_AT_data_bit_offset,                "DW_AT_data_bit_offset" },
  { DW_AT_const_expr,                     "DW_AT_const_expr" },
  { DW_AT_enum_class,                     "DW_AT_enum_class" },
  { DW_AT_linkage_name,                   "DW_AT_linkage_name" },
  { DW_AT_string_length_bit_size,         "DW_AT_string_length_bit_size" },
  { DW_AT_string_length_byte_size,        "DW_AT_string_length_byte_size" },
  { DW_AT_rank,                           "DW_AT_rank" },
  { DW_AT_str_offsets_base,               "DW_AT_str_offsets_base" },
  { DW_AT_addr_base,                      "DW_AT_addr_base" },
  { DW_AT_ranges_base,                    "DW_AT_ranges_base" },
  { DW_AT_dwo_id,                         "DW_AT_dwo_id" },
  { DW_AT_dwo_name,                       "DW_AT_dwo_name" },
  { DW_AT_reference,                      "DW_AT_reference" },
  { DW_AT_rvalue_reference,               "DW_AT_rvalue_reference" },
  { DW_AT_macros,                         "DW_AT_macros" },
  { DW_AT_call_all_calls,                 "DW_AT_call_all_calls" },
  { DW_AT_call_all_source_calls,          "DW_AT_call_all_source_calls" },
  { DW_AT_call_all_tail_calls,            "DW_AT_call_all_tail_calls" },
  { DW_AT_call_return_pc,                 "DW_AT_call_return_pc" },
  { DW_AT_call_value,                     "DW_AT_call_value" },
  { DW_AT_call_origin,                    "DW_AT_call_origin" },
  { DW_AT_call_parameter,                 "DW_AT_call_parameter" },
  { DW_AT_call_pc,                        "DW_AT_call_pc" },
  { DW_AT_call_tail_call,                 "DW_AT_call_tail_call" },
  { DW_AT_call_target,                    "DW_AT_call_target" },
  { DW_AT_call_target_clobbered,          "DW_AT_call_target_clobbered" },
  { DW_AT_call_data_location,             "DW_AT_call_data_location" },
  { DW_AT_call_data_value,                "DW_AT_call_data_value" },
  { DW_AT_noreturn,                       "DW_AT_noreturn" },
  { DW_AT_alignment,                      "DW_AT_alignment" },
  { DW_AT_export_symbols,                 "DW_AT_export_symbols" },
  { DW_AT_deleted,                        "DW_AT_deleted" },
  { DW_AT_defaulted,                      "DW_AT_defaulted" },
  { DW_AT_loclists_base,                  "DW_AT_loclists_base" },
  { DW_AT_HP_block_index,                 "DW_AT_HP_block_index" },
  { DW_AT_lo_user,                        "DW_AT_lo_user" },
  { DW_AT_MIPS_fde,                       "DW_AT_MIPS_fde" },
  { DW_AT_MIPS_loop_begin,                "DW_AT_MIPS_loop_begin" },
  { DW_AT_MIPS_tail_loop_begin,           "DW_AT_MIPS_tail_loop_begin" },
  { DW_AT_MIPS_epilog_begin,              "DW_AT_MIPS_epilog_begin" },
  { DW_AT_MIPS_loop_unroll_factor,        "DW_AT_MIPS_loop_unroll_factor" },
  { DW_AT_MIPS_software_pipeline_depth,   "DW_AT_MIPS_software_pipeline_depth" },
  { DW_AT_MIPS_linkage_name,              "DW_AT_MIPS_linkage_name" },
  { DW_AT_MIPS_stride,                    "DW_AT_MIPS_stride" },
  { DW_AT_MIPS_abstract_name,             "DW_AT_MIPS_abstract_name" },
  { DW_AT_MIPS_clone_origin,              "DW_AT_MIPS_clone_origin" },
  { DW_AT_MIPS_has_inlines,               "DW_AT_MIPS_has_inlines" },
  { DW_AT_MIPS_stride_byte,               "DW_AT_MIPS_stride_byte" },
  { DW_AT_MIPS_stride_elem,               "DW_AT_MIPS_stride_elem" },
  { DW_AT_MIPS_ptr_dopetype,              "DW_AT_MIPS_ptr_dopetype" },
  { DW_AT_MIPS_allocatable_dopetype,      "DW_AT_MIPS_allocatable_dopetype" },
  { DW_AT_MIPS_assumed_shape_dopetype,    "DW_AT_MIPS_assumed_shape_dopetype" },
  { DW_AT_MIPS_assumed_size,              "DW_AT_MIPS_assumed_size" },
  { DW_AT_HP_unmodifiable,                "DW_AT_HP_unmodifiable" },
  { DW_AT_HP_actuals_stmt_list,           "DW_AT_HP_actuals_stmt_list" },
  { DW_AT_HP_proc_per_section,            "DW_AT_HP_proc_per_section" },
  { DW_AT_HP_raw_data_ptr,                "DW_AT_HP_raw_data_ptr" },
  { DW_AT_HP_pass_by_reference,           "DW_AT_HP_pass_by_reference" },
  { DW_AT_HP_opt_level,                   "DW_AT_HP_opt_level" },
  { DW_AT_HP_prof_version_id,             "DW_AT_HP_prof_version_id" },
  { DW_AT_HP_opt_flags,                   "DW_AT_HP_opt_flags" },
  { DW_AT_HP_cold_region_low_pc,          "DW_AT_HP_cold_region_low_pc" },
  { DW_AT_HP_cold_region_high_pc,         "DW_AT_HP_cold_region_high_pc" },
  { DW_AT_HP_all_variables_modifiable,    "DW_AT_HP_all_variables_modifiable" },
  { DW_AT_HP_linkage_name,                "DW_AT_HP_linkage_name" },
  { DW_AT_HP_prof_flags,                  "DW_AT_HP_prof_flags" },
  { DW_AT_CPQ_discontig_ranges,           "DW_AT_CPQ_discontig_ranges" },
  { DW_AT_CPQ_semantic_events,            "DW_AT_CPQ_semantic_events" },
  { DW_AT_CPQ_split_lifetimes_var,        "DW_AT_CPQ_split_lifetimes_var" },
  { DW_AT_CPQ_split_lifetimes_rtn,        "DW_AT_CPQ_split_lifetimes_rtn" },
  { DW_AT_CPQ_prologue_length,            "DW_AT_CPQ_prologue_length" },
  { DW_AT_INTEL_other_endian,             "DW_AT_INTEL_other_endian" },
  { DW_AT_sf_names,                       "DW_AT_sf_names" },
  { DW_AT_src_info,                       "DW_AT_src_info" },
  { DW_AT_mac_info,                       "DW_AT_mac_info" },
  { DW_AT_src_coords,                     "DW_AT_src_coords" },
  { DW_AT_body_begin,                     "DW_AT_body_begin" },
  { DW_AT_body_end,                       "DW_AT_body_end" },
  { DW_AT_GNU_vector,                     "DW_AT_GNU_vector" },
  { DW_AT_GNU_guarded_by,                 "DW_AT_GNU_guarded_by" },
  { DW_AT_GNU_pt_guarded_by,              "DW_AT_GNU_pt_guarded_by" },
  { DW_AT_GNU_guarded,                    "DW_AT_GNU_guarded" },
  { DW_AT_GNU_pt_guarded,                 "DW_AT_GNU_pt_guarded" },
  { DW_AT_GNU_locks_excluded,             "DW_AT_GNU_locks_excluded" },
  { DW_AT_GNU_exclusive_locks_required,   "DW_AT_GNU_exclusive_locks_required" },
  { DW_AT_GNU_shared_locks_required,      "DW_AT_GNU_shared_locks_required" },
  { DW_AT_GNU_odr_signature,              "DW_AT_GNU_odr_signature" },
  { DW_AT_GNU_template_name,              "DW_AT_GNU_template_name" },
  { DW_AT_GNU_call_site_value,            "DW_AT_GNU_call_site_value" },
  { DW_AT_GNU_call_site_data_value,       "DW_AT_GNU_call_site_data_value" },
  { DW_AT_GNU_call_site_target,           "DW_AT_GNU_call_site_target" },
  { DW_AT_GNU_call_site_target_clobbered, "DW_AT_GNU_call_site_target_clobbered" },
  { DW_AT_GNU_tail_call,                  "DW_AT_GNU_tail_call" },
  { DW_AT_GNU_all_tail_call_sites,        "DW_AT_GNU_all_tail_call_sites" },
  { DW_AT_GNU_all_call_sites,             "DW_AT_GNU_all_call_sites" },
  { DW_AT_GNU_all_source_call_sites,      "DW_AT_GNU_all_source_call_sites" },
  { DW_AT_GNU_macros,                     "DW_AT_GNU_macros" },
  { DW_AT_GNU_dwo_name,                   "DW_AT_GNU_dwo_name" },
  { DW_AT_GNU_dwo_id,                     "DW_AT_GNU_dwo_id" },
  { DW_AT_GNU_ranges_base,                "DW_AT_GNU_ranges_base" },
  { DW_AT_GNU_addr_base,                  "DW_AT_GNU_addr_base" },
  { DW_AT_GNU_pubnames,                   "DW_AT_GNU_pubnames" },
  { DW_AT_GNU_pubtypes,                   "DW_AT_GNU_pubtypes" },
  { DW_AT_GNU_discriminator,              "DW_AT_GNU_discriminator" },
  { DW_AT_GNU_numerator,                  "DW_AT_GNU_numerator" },
  { DW_AT_GNU_denominator,                "DW_AT_GNU_denominator" },
  { DW_AT_GNU_bias,                       "DW_AT_GNU_bias" },
  { DW_AT_ALTIUM_loclist,                 "DW_AT_ALTIUM_loclist" },
  { DW_AT_SUN_template,                   "DW_AT_SUN_template" },
  { DW_AT_VMS_rtnbeg_pd_address,          "DW_AT_VMS_rtnbeg_pd_address" },
  { DW_AT_SUN_alignment,                  "DW_AT_SUN_alignment" },
  { DW_AT_SUN_vtable,                     "DW_AT_SUN_vtable" },
  { DW_AT_SUN_count_guarantee,            "DW_AT_SUN_count_guarantee" },
  { DW_AT_SUN_command_line,               "DW_AT_SUN_command_line" },
  { DW_AT_SUN_vbase,                      "DW_AT_SUN_vbase" },
  { DW_AT_SUN_compile_options,            "DW_AT_SUN_compile_options" },
  { DW_AT_SUN_language,                   "DW_AT_SUN_language" },
  { DW_AT_SUN_browser_file,               "DW_AT_SUN_browser_file" },
  { DW_AT_SUN_vtable_abi,                 "DW_AT_SUN_vtable_abi" },
  { DW_AT_SUN_func_offsets,               "DW_AT_SUN_func_offsets" },
  { DW_AT_SUN_cf_kind,                    "DW_AT_SUN_cf_kind" },
  { DW_AT_SUN_vtable_index,               "DW_AT_SUN_vtable_index" },
  { DW_AT_SUN_omp_tpriv_addr,             "DW_AT_SUN_omp_tpriv_addr" },
  { DW_AT_SUN_omp_child_func,             "DW_AT_SUN_omp_child_func" },
  { DW_AT_SUN_func_offset,                "DW_AT_SUN_func_offset" },
  { DW_AT_SUN_memop_type_ref,             "DW_AT_SUN_memop_type_ref" },
  { DW_AT_SUN_profile_id,                 "DW_AT_SUN_profile_id" },
  { DW_AT_SUN_memop_signature,            "DW_AT_SUN_memop_signature" },
  { DW_AT_SUN_obj_dir,                    "DW_AT_SUN_obj_dir" },
  { DW_AT_SUN_obj_file,                   "DW_AT_SUN_obj_file" },
  { DW_AT_SUN_original_name,              "DW_AT_SUN_original_name" },
  { DW_AT_SUN_hwcprof_signature,          "DW_AT_SUN_hwcprof_signature" },
  { DW_AT_SUN_amd64_parmdump,             "DW_AT_SUN_amd64_parmdump" },
  { DW_AT_SUN_part_link_name,             "DW_AT_SUN_part_link_name" },
  { DW_AT_SUN_link_name,                  "DW_AT_SUN_link_name" },
  { DW_AT_SUN_pass_with_const,            "DW_AT_SUN_pass_with_const" },
  { DW_AT_SUN_return_with_const,          "DW_AT_SUN_return_with_const" },
  { DW_AT_SUN_import_by_name,             "DW_AT_SUN_import_by_name" },
  { DW_AT_SUN_f90_pointer,                "DW_AT_SUN_f90_pointer" },
  { DW_AT_SUN_pass_by_ref,                "DW_AT_SUN_pass_by_ref" },
  { DW_AT_SUN_f90_allocatable,            "DW_AT_SUN_f90_allocatable" },
  { DW_AT_SUN_f90_assumed_shape_array,    "DW_AT_SUN_f90_assumed_shape_array" },
  { DW_AT_SUN_c_vla,                      "DW_AT_SUN_c_vla" },
  { DW_AT_SUN_return_value_ptr,           "DW_AT_SUN_return_value_ptr" },
  { DW_AT_SUN_dtor_start,                 "DW_AT_SUN_dtor_start" },
  { DW_AT_SUN_dtor_length,                "DW_AT_SUN_dtor_length" },
  { DW_AT_SUN_dtor_state_initial,         "DW_AT_SUN_dtor_state_initial" },
  { DW_AT_SUN_dtor_state_final,           "DW_AT_SUN_dtor_state_final" },
  { DW_AT_SUN_dtor_state_deltas,          "DW_AT_SUN_dtor_state_deltas" },
  { DW_AT_SUN_import_by_lname,            "DW_AT_SUN_import_by_lname" },
  { DW_AT_SUN_f90_use_only,               "DW_AT_SUN_f90_use_only" },
  { DW_AT_SUN_namelist_spec,              "DW_AT_SUN_namelist_spec" },
  { DW_AT_SUN_is_omp_child_func,          "DW_AT_SUN_is_omp_child_func" },
  { DW_AT_SUN_fortran_main_alias,         "DW_AT_SUN_fortran_main_alias" },
  { DW_AT_SUN_fortran_based,              "DW_AT_SUN_fortran_based" },
  { DW_AT_use_GNAT_descriptive_type,      "DW_AT_use_GNAT_descriptive_type" },
  { DW_AT_GNAT_descriptive_type,          "DW_AT_GNAT_descriptive_type" },
  { DW_AT_upc_threads_scaled,             "DW_AT_upc_threads_scaled" },
  { DW_AT_PGI_lbase,                      "DW_AT_PGI_lbase" },
  { DW_AT_PGI_soffset,                    "DW_AT_PGI_soffset" },
  { DW_AT_PGI_lstride,                    "DW_AT_PGI_lstride" },
  { DW_AT_APPLE_optimized,                "DW_AT_APPLE_optimized" },
  { DW_AT_APPLE_flags,                    "DW_AT_APPLE_flags" },
  { DW_AT_APPLE_isa,                      "DW_AT_APPLE_isa" },
  { DW_AT_APPLE_block,                    "DW_AT_APPLE_block" },
  { DW_AT_APPLE_major_runtime_vers,       "DW_AT_APPLE_major_runtime_vers" },
  { DW_AT_APPLE_runtime_class,            "DW_AT_APPLE_runtime_class" },
  { DW_AT_APPLE_omit_frame_ptr,           "DW_AT_APPLE_omit_frame_ptr" },
  { DW_AT_APPLE_closure,                  "DW_AT_APPLE_closure" },
  { DW_AT_APPLE_major_runtime_vers,       "DW_AT_APPLE_major_runtime_vers" },
  { DW_AT_APPLE_runtime_class,            "DW_AT_APPLE_runtime_class" },
  { 0, NULL },
};

static id2Str_t DW_ATE_stringInfos [] = {
  { DW_ATE_address,         "DW_ATE_address" },
  { DW_ATE_boolean,         "DW_ATE_boolean" },
  { DW_ATE_complex_float,   "DW_ATE_complex_float" },
  { DW_ATE_float,           "DW_ATE_float" },
  { DW_ATE_signed,          "DW_ATE_signed" },
  { DW_ATE_signed_char,     "DW_ATE_signed_char" },
  { DW_ATE_unsigned,        "DW_ATE_unsigned" },
  { DW_ATE_unsigned_char,   "DW_ATE_unsigned_char" },
  { DW_ATE_imaginary_float, "DW_ATE_imaginary_float" },
  { DW_ATE_packed_decimal,  "DW_ATE_packed_decimal" },
  { DW_ATE_numeric_string,  "DW_ATE_numeric_string" },
  { DW_ATE_edited,          "DW_ATE_edited" },
  { DW_ATE_signed_fixed,    "DW_ATE_signed_fixed" },
  { DW_ATE_unsigned_fixed,  "DW_ATE_unsigned_fixed" },
  { DW_ATE_decimal_float,   "DW_ATE_decimal_float" },
  { DW_ATE_UTF,             "DW_ATE_UTF" },
  { DW_ATE_UCS,             "DW_ATE_UCS" },
  { DW_ATE_ASCII,           "DW_ATE_ASCII" },
  { 0, NULL },
};

static id2Str_t DW_OP_stringInfos [] = {
  { DW_OP_addr,                 "DW_OP_addr" },
  { DW_OP_deref,                "DW_OP_deref" },
  { DW_OP_const1u,              "DW_OP_const1u" },
  { DW_OP_const1s,              "DW_OP_const1s" },
  { DW_OP_const2u,              "DW_OP_const2u" },
  { DW_OP_const2s,              "DW_OP_const2s" },
  { DW_OP_const4u,              "DW_OP_const4u" },
  { DW_OP_const4s,              "DW_OP_const4s" },
  { DW_OP_const8u,              "DW_OP_const8u" },
  { DW_OP_const8s,              "DW_OP_const8s" },
  { DW_OP_constu,               "DW_OP_constu" },
  { DW_OP_consts,               "DW_OP_consts" },
  { DW_OP_dup,                  "DW_OP_dup" },
  { DW_OP_drop,                 "DW_OP_drop" },
  { DW_OP_over,                 "DW_OP_over" },
  { DW_OP_pick,                 "DW_OP_pick" },
  { DW_OP_swap,                 "DW_OP_swap" },
  { DW_OP_rot,                  "DW_OP_rot" },
  { DW_OP_xderef,               "DW_OP_xderef" },
  { DW_OP_abs,                  "DW_OP_abs" },
  { DW_OP_and,                  "DW_OP_and" },
  { DW_OP_div,                  "DW_OP_div" },
  { DW_OP_minus,                "DW_OP_minus" },
  { DW_OP_mod,                  "DW_OP_mod" },
  { DW_OP_mul,                  "DW_OP_mul" },
  { DW_OP_neg,                  "DW_OP_neg" },
  { DW_OP_not,                  "DW_OP_not" },
  { DW_OP_or,                   "DW_OP_or" },
  { DW_OP_plus,                 "DW_OP_plus" },
  { DW_OP_plus_uconst,          "DW_OP_plus_uconst" },
  { DW_OP_shl,                  "DW_OP_shl" },
  { DW_OP_shr,                  "DW_OP_shr" },
  { DW_OP_shra,                 "DW_OP_shra" },
  { DW_OP_xor,                  "DW_OP_xor" },
  { DW_OP_bra,                  "DW_OP_bra" },
  { DW_OP_eq,                   "DW_OP_eq" },
  { DW_OP_ge,                   "DW_OP_ge" },
  { DW_OP_gt,                   "DW_OP_gt" },
  { DW_OP_le,                   "DW_OP_le" },
  { DW_OP_lt,                   "DW_OP_lt" },
  { DW_OP_ne,                   "DW_OP_ne" },
  { DW_OP_skip,                 "DW_OP_skip" },
  { DW_OP_lit0,                 "DW_OP_lit0" },
  { DW_OP_lit1,                 "DW_OP_lit1" },
  { DW_OP_lit2,                 "DW_OP_lit2" },
  { DW_OP_lit3,                 "DW_OP_lit3" },
  { DW_OP_lit4,                 "DW_OP_lit4" },
  { DW_OP_lit5,                 "DW_OP_lit5" },
  { DW_OP_lit6,                 "DW_OP_lit6" },
  { DW_OP_lit7,                 "DW_OP_lit7" },
  { DW_OP_lit8,                 "DW_OP_lit8" },
  { DW_OP_lit9,                 "DW_OP_lit9" },
  { DW_OP_lit10,                "DW_OP_lit10" },
  { DW_OP_lit11,                "DW_OP_lit11" },
  { DW_OP_lit12,                "DW_OP_lit12" },
  { DW_OP_lit13,                "DW_OP_lit13" },
  { DW_OP_lit14,                "DW_OP_lit14" },
  { DW_OP_lit15,                "DW_OP_lit15" },
  { DW_OP_lit16,                "DW_OP_lit16" },
  { DW_OP_lit17,                "DW_OP_lit17" },
  { DW_OP_lit18,                "DW_OP_lit18" },
  { DW_OP_lit19,                "DW_OP_lit19" },
  { DW_OP_lit20,                "DW_OP_lit20" },
  { DW_OP_lit21,                "DW_OP_lit21" },
  { DW_OP_lit22,                "DW_OP_lit22" },
  { DW_OP_lit23,                "DW_OP_lit23" },
  { DW_OP_lit24,                "DW_OP_lit24" },
  { DW_OP_lit25,                "DW_OP_lit25" },
  { DW_OP_lit26,                "DW_OP_lit26" },
  { DW_OP_lit27,                "DW_OP_lit27" },
  { DW_OP_lit28,                "DW_OP_lit28" },
  { DW_OP_lit29,                "DW_OP_lit29" },
  { DW_OP_lit30,                "DW_OP_lit30" },
  { DW_OP_lit31,                "DW_OP_lit31" },
  { DW_OP_reg0,                 "DW_OP_reg0" },
  { DW_OP_reg1,                 "DW_OP_reg1" },
  { DW_OP_reg2,                 "DW_OP_reg2" },
  { DW_OP_reg3,                 "DW_OP_reg3" },
  { DW_OP_reg4,                 "DW_OP_reg4" },
  { DW_OP_reg5,                 "DW_OP_reg5" },
  { DW_OP_reg6,                 "DW_OP_reg6" },
  { DW_OP_reg7,                 "DW_OP_reg7" },
  { DW_OP_reg8,                 "DW_OP_reg8" },
  { DW_OP_reg9,                 "DW_OP_reg9" },
  { DW_OP_reg10,                "DW_OP_reg10" },
  { DW_OP_reg11,                "DW_OP_reg11" },
  { DW_OP_reg12,                "DW_OP_reg12" },
  { DW_OP_reg13,                "DW_OP_reg13" },
  { DW_OP_reg14,                "DW_OP_reg14" },
  { DW_OP_reg15,                "DW_OP_reg15" },
  { DW_OP_reg16,                "DW_OP_reg16" },
  { DW_OP_reg17,                "DW_OP_reg17" },
  { DW_OP_reg18,                "DW_OP_reg18" },
  { DW_OP_reg19,                "DW_OP_reg19" },
  { DW_OP_reg20,                "DW_OP_reg20" },
  { DW_OP_reg21,                "DW_OP_reg21" },
  { DW_OP_reg22,                "DW_OP_reg22" },
  { DW_OP_reg23,                "DW_OP_reg23" },
  { DW_OP_reg24,                "DW_OP_reg24" },
  { DW_OP_reg25,                "DW_OP_reg25" },
  { DW_OP_reg26,                "DW_OP_reg26" },
  { DW_OP_reg27,                "DW_OP_reg27" },
  { DW_OP_reg28,                "DW_OP_reg28" },
  { DW_OP_reg29,                "DW_OP_reg29" },
  { DW_OP_reg30,                "DW_OP_reg30" },
  { DW_OP_reg31,                "DW_OP_reg31" },
  { DW_OP_breg0,                "DW_OP_breg0" },
  { DW_OP_breg1,                "DW_OP_breg1" },
  { DW_OP_breg2,                "DW_OP_breg2" },
  { DW_OP_breg3,                "DW_OP_breg3" },
  { DW_OP_breg4,                "DW_OP_breg4" },
  { DW_OP_breg5,                "DW_OP_breg5" },
  { DW_OP_breg6,                "DW_OP_breg6" },
  { DW_OP_breg7,                "DW_OP_breg7" },
  { DW_OP_breg8,                "DW_OP_breg8" },
  { DW_OP_breg9,                "DW_OP_breg9" },
  { DW_OP_breg10,               "DW_OP_breg10" },
  { DW_OP_breg11,               "DW_OP_breg11" },
  { DW_OP_breg12,               "DW_OP_breg12" },
  { DW_OP_breg13,               "DW_OP_breg13" },
  { DW_OP_breg14,               "DW_OP_breg14" },
  { DW_OP_breg15,               "DW_OP_breg15" },
  { DW_OP_breg16,               "DW_OP_breg16" },
  { DW_OP_breg17,               "DW_OP_breg17" },
  { DW_OP_breg18,               "DW_OP_breg18" },
  { DW_OP_breg19,               "DW_OP_breg19" },
  { DW_OP_breg20,               "DW_OP_breg20" },
  { DW_OP_breg21,               "DW_OP_breg21" },
  { DW_OP_breg22,               "DW_OP_breg22" },
  { DW_OP_breg23,               "DW_OP_breg23" },
  { DW_OP_breg24,               "DW_OP_breg24" },
  { DW_OP_breg25,               "DW_OP_breg25" },
  { DW_OP_breg26,               "DW_OP_breg26" },
  { DW_OP_breg27,               "DW_OP_breg27" },
  { DW_OP_breg28,               "DW_OP_breg28" },
  { DW_OP_breg29,               "DW_OP_breg29" },
  { DW_OP_breg30,               "DW_OP_breg30" },
  { DW_OP_breg31,               "DW_OP_breg31" },
  { DW_OP_regx,                 "DW_OP_regx" },
  { DW_OP_fbreg,                "DW_OP_fbreg" },
  { DW_OP_bregx,                "DW_OP_bregx" },
  { DW_OP_piece,                "DW_OP_piece" },
  { DW_OP_deref_size,           "DW_OP_deref_size" },
  { DW_OP_xderef_size,          "DW_OP_xderef_size" },
  { DW_OP_nop,                  "DW_OP_nop" },
  { DW_OP_push_object_address,  "DW_OP_push_object_address" },
  { DW_OP_call2,                "DW_OP_call2" },
  { DW_OP_call4,                "DW_OP_call4" },
  { DW_OP_call_ref,             "DW_OP_call_ref" },
  { DW_OP_form_tls_address,     "DW_OP_form_tls_address" },
  { DW_OP_call_frame_cfa,       "DW_OP_call_frame_cfa" },
  { DW_OP_bit_piece,            "DW_OP_bit_piece" },
  { DW_OP_implicit_value,       "DW_OP_implicit_value" },
  { DW_OP_stack_value,          "DW_OP_stack_value" },
  { DW_OP_implicit_pointer,     "DW_OP_implicit_pointer" },
  { DW_OP_addrx,                "DW_OP_addrx" },
  { DW_OP_constx,               "DW_OP_constx" },
  { DW_OP_entry_value,          "DW_OP_entry_value" },
  { DW_OP_const_type,           "DW_OP_const_type" },
  { DW_OP_regval_type,          "DW_OP_regval_type" },
  { DW_OP_deref_type,           "DW_OP_deref_type" },
  { DW_OP_xderef_type,          "DW_OP_xderef_type" },
  { DW_OP_convert,              "DW_OP_convert" },
  { DW_OP_reinterpret,          "DW_OP_reinterpret" },
  { DW_OP_GNU_push_tls_address, "DW_OP_GNU_push_tls_address" },
  { DW_OP_lo_user,              "DW_OP_lo_user" },
  { DW_OP_GNU_uninit,           "DW_OP_GNU_uninit" },
  { DW_OP_GNU_encoded_addr,     "DW_OP_GNU_encoded_addr" },
  { DW_OP_GNU_implicit_pointer, "DW_OP_GNU_implicit_pointer" },
  { DW_OP_GNU_entry_value,      "DW_OP_GNU_entry_value" },
  { DW_OP_GNU_const_type,       "DW_OP_GNU_const_type" },
  { DW_OP_GNU_regval_type,      "DW_OP_GNU_regval_type" },
  { DW_OP_GNU_deref_type,       "DW_OP_GNU_deref_type" },
  { DW_OP_GNU_convert,          "DW_OP_GNU_convert" },
  { DW_OP_GNU_reinterpret,      "DW_OP_GNU_reinterpret" },
  { DW_OP_GNU_parameter_ref,    "DW_OP_GNU_parameter_ref" },
  { DW_OP_GNU_addr_index,       "DW_OP_GNU_addr_index" },
  { DW_OP_GNU_const_index,      "DW_OP_GNU_const_index" },
  { DW_OP_HP_unknown,           "DW_OP_HP_unknown" },
  { DW_OP_HP_is_value,          "DW_OP_HP_is_value" },
  { DW_OP_HP_fltconst4,         "DW_OP_HP_fltconst4" },
  { DW_OP_HP_fltconst8,         "DW_OP_HP_fltconst8" },
  { DW_OP_HP_mod_range,         "DW_OP_HP_mod_range" },
  { DW_OP_HP_unmod_range,       "DW_OP_HP_unmod_range" },
  { DW_OP_HP_tls,               "DW_OP_HP_tls" },
  { DW_OP_INTEL_bit_piece,      "DW_OP_INTEL_bit_piece" },
  { DW_OP_APPLE_uninit,         "DW_OP_APPLE_uninit" },
  { DW_OP_PGI_omp_thread_num,   "DW_OP_PGI_omp_thread_num" },
  { DW_OP_hi_user,              "DW_OP_hi_user" },
  { 0, NULL },
};

static id2Str_t DW_INL_stringInfos [] = {
  { DW_INL_not_inlined,          "DW_INL_not_inlined" },
  { DW_INL_inlined,              "DW_INL_inlined" },
  { DW_INL_declared_not_inlined, "DW_INL_declared_not_inlined" },
  { DW_INL_declared_inlined,     "DW_INL_declared_inlined" },
  { 0, NULL },
};

static id2Str_t DW_RANGES_TYPE_stringInfos [] = {
  { DW_RANGES_ENTRY,             "DW_RANGES_ENTRY" },
  { DW_RANGES_ADDRESS_SELECTION, "DW_RANGES_ADDRESS_SELECTION" },
  { DW_RANGES_END,               "DW_RANGES_END" },
  { 0, NULL },
};

// =================================== getDW_TAG_string =========================== 

static uint8_t getDW_TAG_string(dwarfDbgPtr_t self, Dwarf_Half tag, const char **string) {
  id2Str_t *entry;
  
  entry = &DW_TAG_stringInfos[0];
  while (entry->str != NULL) {
    if (tag == entry->id) {
      *string = entry->str;
      return DWARF_DBG_ERR_OK;
    }
    entry++;
  }
  return DWARF_DBG_ERR_DW_TAG_STRING_NOT_FOUND;
}

// =================================== getDW_FORM_string =========================== 

static uint8_t getDW_FORM_string(dwarfDbgPtr_t self, Dwarf_Half theform, const char **string) {
  id2Str_t *entry;
  
  entry = &DW_FORM_stringInfos[0];
  while (entry->str != NULL) {
    if (theform == entry->id) {
      *string = entry->str;
      return DWARF_DBG_ERR_OK;
    }
    entry++;
  }
  return DWARF_DBG_ERR_DW_FORM_STRING_NOT_FOUND;
}

// =================================== getDW_AT_string =========================== 

static uint8_t getDW_AT_string(dwarfDbgPtr_t self, Dwarf_Half attr, const char **string) {
  id2Str_t *entry;
  
  entry = &DW_AT_stringInfos[0];
  while (entry->str != NULL) {
    if (attr == entry->id) {
      *string = entry->str;
      return DWARF_DBG_ERR_OK;
    }
    entry++;
  }
  return DWARF_DBG_ERR_DW_AT_STRING_NOT_FOUND;
}

// =================================== getDW_ATE_string =========================== 

static uint8_t getDW_ATE_string(dwarfDbgPtr_t self, Dwarf_Half attr, const char **string) {
  id2Str_t *entry;
  
  entry = &DW_ATE_stringInfos[0];
  while (entry->str != NULL) {
    if (attr == entry->id) {
      *string = entry->str;
      return DWARF_DBG_ERR_OK;
    }
    entry++;
  }
  return DWARF_DBG_ERR_DW_ATE_STRING_NOT_FOUND;
}

// =================================== getDW_OP_string =========================== 

static uint8_t getDW_OP_string(dwarfDbgPtr_t self, Dwarf_Small op, const char **string) {
  id2Str_t *entry;
  
  entry = &DW_OP_stringInfos[0];
  while (entry->str != NULL) {
    if (op == entry->id) {
      *string = entry->str;
      return DWARF_DBG_ERR_OK;
    }
    entry++;
  }
  return DWARF_DBG_ERR_DW_OP_STRING_NOT_FOUND;
}

// =================================== getDW_INL_string =========================== 

static uint8_t getDW_INL_string(dwarfDbgPtr_t self, Dwarf_Unsigned inl, const char **string) {
  id2Str_t *entry;
  
  entry = &DW_INL_stringInfos[0];
  while (entry->str != NULL) {
    if (inl == entry->id) {
      *string = entry->str;
      return DWARF_DBG_ERR_OK;
    }
    entry++;
  }
  return DWARF_DBG_ERR_DW_OP_STRING_NOT_FOUND;
}

// =================================== getDW_RANGES_TYPE_string =========================== 

static uint8_t getDW_RANGES_TYPE_string(dwarfDbgPtr_t self, int dwrType, const char **string) {
  id2Str_t *entry;
  
  entry = &DW_RANGES_TYPE_stringInfos[0];
  while (entry->str != NULL) {
    if (dwrType == entry->id) {
      *string = entry->str;
      return DWARF_DBG_ERR_OK;
    }
    entry++;
  }
  return DWARF_DBG_ERR_DW_OP_STRING_NOT_FOUND;
}

// =================================== dwarfDbgStringInfoInit =========================== 

int dwarfDbgStringInfoInit (dwarfDbgPtr_t self) {

  self->dwarfDbgStringInfo->getDW_TAG_string = &getDW_TAG_string;
  self->dwarfDbgStringInfo->getDW_FORM_string = &getDW_FORM_string;
  self->dwarfDbgStringInfo->getDW_AT_string = &getDW_AT_string;
  self->dwarfDbgStringInfo->getDW_ATE_string = &getDW_ATE_string;
  self->dwarfDbgStringInfo->getDW_OP_string = &getDW_OP_string;
  self->dwarfDbgStringInfo->getDW_INL_string = &getDW_INL_string;
  self->dwarfDbgStringInfo->getDW_RANGES_TYPE_string = &getDW_RANGES_TYPE_string;
  return DWARF_DBG_ERR_OK;
}

