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
 * File:   dwarfDbgOpenElf.c
 * Author: Arnulf P. Wiedemann <arnulf@wiedemann-pri.de>
 *
 * Created on January 22, 2017
 */

#include <tcl.h>
#include "dwarfDbgInt.h"

static dwarfDbgEsb_t config_file_path;
static dwarfDbgEsb_t config_file_tiedpath;
static dwarfDbgEsb_t esbShortCuName;
static dwarfDbgEsb_t esbLongCuName;
static dwarfDbgEsb_t dwarf_error_line;

/*  Base Address is needed for range lists and must come from a CU.
  Low address is for information and can come from a function
  or something in the CU. */
Dwarf_Addr CuBaseAddress = 0;     /* CU Base address */
Dwarf_Addr CuLowAddress = 0;    /* CU low address */
Dwarf_Addr CuHighAddress = 0;     /* CU High address */

Dwarf_Off  DIEOffset = 0;      /* DIE offset in compile unit */
Dwarf_Off  DIEOverallOffset = 0;  /* DIE offset in .debug_info */

/*  These globals  enable better error reporting. */
Dwarf_Off  DIECuOffset = 0;     /* CU DIE offset in compile unit */
Dwarf_Off  DIECuOverallOffset = 0; /* CU DIE offset in .debug_info */
dwarfDbgPtr_t _self;


// =================================== get_attr_value =========================== 

/*  Fill buffer with attribute value.
  We pass in tag so we can try to do the right thing with
  broken compiler DW_TAG_enumerator

  'cnt' is signed for historical reasons (a mistake
  in an interface), but the value is never negative.

  We append to esbp's buffer.
*/
void get_attr_value(Dwarf_Debug dbg, Dwarf_Half tag, Dwarf_Die die, Dwarf_Off dieprintCuGoffset,
  Dwarf_Attribute attrib, char **srcfiles, Dwarf_Signed cnt, dwarfDbgEsb_t *esbp, int show_form, int local_verbose) {
  Dwarf_Half theform = 0;
  char * temps = 0;
  Dwarf_Block *tempb = 0;
  Dwarf_Signed tempsd = 0;
  Dwarf_Unsigned tempud = 0;
  Dwarf_Off off = 0;
  Dwarf_Die die_for_check = 0;
  Dwarf_Half tag_for_check = 0;
  Dwarf_Bool tempbool = 0;
  Dwarf_Addr addr = 0;
  int fres = 0;
  int bres = 0;
  int wres = 0;
  int dres = 0;
  Dwarf_Half direct_form = 0;
  char small_buf[COMPILE_UNIT_NAME_LEN];  /* Size to hold a filename */
  Dwarf_Bool is_info = TRUE;
  Dwarf_Error err = 0;
  dwarfDbgPtr_t self = _self;


  is_info = dwarf_get_die_infotypes_flag(die);
  /*  Dwarf_whatform gets the real form, DW_FORM_indir is
    never returned: instead the real form following
    DW_FORM_indir is returned. */
  fres = dwarf_whatform(attrib, &theform, &err);
  /*  Depending on the form and the attribute, process the form. */
  if (fres == DW_DLV_ERROR) {
    printf("dwarf_whatform cannot Find Attr Form fres: %d err: %d", fres, err);
  } else if (fres == DW_DLV_NO_ENTRY) {
    return;
  }
  /*  dwarf_whatform_direct gets the 'direct' form, so if
    the form is DW_FORM_indir that is what is returned. */
  dwarf_whatform_direct(attrib, &direct_form, &err);
  /*  Ignore errors in dwarf_whatform_direct() */

  switch (theform) {
  case DW_FORM_GNU_addr_index:
  case DW_FORM_addrx:
  case DW_FORM_addr:
    bres = dwarf_formaddr(attrib, &addr, &err);
    if (bres == DW_DLV_OK) {
      if (theform == DW_FORM_GNU_addr_index ||
        theform == DW_FORM_addrx) {
        Dwarf_Unsigned index = 0;
        int res = dwarf_get_debug_addr_index(attrib,&index,&err);
        if(res != DW_DLV_OK) {
          printf("addr missing index ?! res: %d err: %d", res, err);
        }
#ifdef NOTDEF
        bracket_hex("(addr_index: ",index, ")",esbp);
#endif
      }
#ifdef NOTDEF
      bracket_hex("",addr,"",esbp);
#endif
    } else if (bres == DW_DLV_ERROR) {
      if (DW_DLE_MISSING_NEEDED_DEBUG_ADDR_SECTION ==
        dwarf_errno(err)) {
        Dwarf_Unsigned index = 0;
        int res = dwarf_get_debug_addr_index(attrib,&index,&err);
        if(res != DW_DLV_OK) {
          printf("addr missing index ?! bres: %d err: %d", bres, err);
        }

        addr = 0;
#ifdef NOTDEF
        bracket_hex("(addr_index: ",index,
          ")<no .debug_addr section>",esbp);
#endif
        /*  This is normal in a .dwo file. The .debug_addr
          is in a .o and in the final executable. */
      } else {
        printf("addr form with no addr?! bres: %d err: %d", bres, err);
      }
    } else {
      printf("addr is a DW_DLV_NO_ENTRY? Impossible. bres: %d err: %d", bres, err);
    }
    break;
  case DW_FORM_ref_addr:
    {
    Dwarf_Half attr = 0;
    /*  DW_FORM_ref_addr is not accessed thru formref: ** it is an
      address (global section offset) in ** the .debug_info
      section. */
    bres = dwarf_global_formref(attrib, &off, &err);
    if (bres == DW_DLV_OK) {
#ifdef NOTDEF
      bracket_hex("<GOFF=",off, ">",esbp);
#endif
    } else {
      printf("DW_FORM_ref_addr form with no reference?! bres: %d err: %d", bres, err);
    }
    wres = dwarf_whatattr(attrib, &attr, &err);
    if (wres == DW_DLV_ERROR) {
    } else if (wres == DW_DLV_NO_ENTRY) {
    } else {
      if (attr == DW_AT_sibling) {
        /*  The value had better be inside the current CU
          else there is a nasty error here, as a sibling
          has to be in the same CU, it seems. */
        /*  The target offset (off) had better be
          following the die's global offset else
          we have a serious botch. this FORM
          defines the value as a .debug_info
          global offset. */
        Dwarf_Off cuoff = 0;
        Dwarf_Off culen = 0;
        Dwarf_Off die_overall_offset = 0;
        int res = 0;
        int ores = dwarf_dieoffset(die, &die_overall_offset, &err);
        if (ores != DW_DLV_OK) {
          printf("dwarf_dieoffset ores: %d err: %d", ores, err);
        }
#ifdef NOTDEF
        SET_DIE_STACK_SIBLING(off);
        if (die_overall_offset >= off) {
          snprintf(small_buf,sizeof(small_buf),
            "ERROR: Sibling DW_FORM_ref_offset 0x%"
            DW_PR_XZEROS DW_PR_DUx
            " points %s die Global offset "
            "0x%"  DW_PR_XZEROS  DW_PR_DUx,
            off,(die_overall_offset == off)?"at":"before",
            die_overall_offset);
          printf(small_buf,DW_DLV_OK,0);
        }
#endif
        res = dwarf_die_CU_offset_range(die,&cuoff,
          &culen,&err);
//        DWARF_CHECK_COUNT(tag_tree_result,1);
        if (res != DW_DLV_OK) {
        } else {
          Dwarf_Off cuend = cuoff+culen;
          if (off <  cuoff || off >= cuend) {
//            DWARF_CHECK_ERROR(tag_tree_result,
//              "DW_AT_sibling DW_FORM_ref_addr offset points "
//              "outside of current CU");
          }
        }
      }
    }
    }

    break;
  case DW_FORM_ref1:
  case DW_FORM_ref2:
  case DW_FORM_ref4:
  case DW_FORM_ref8:
  case DW_FORM_ref_udata:
    {
    int refres = 0;
    Dwarf_Half attr = 0;
    Dwarf_Off goff = 0; /* Global offset */
    Dwarf_Error referr = 0;

    /* CU-relative offset returned. */
    refres = dwarf_formref(attrib, &off, &referr);
    if (refres != DW_DLV_OK) {
      /* Report incorrect offset */
      snprintf(small_buf,sizeof(small_buf),
        "%s, offset=<0x%"  DW_PR_XZEROS  DW_PR_DUx
        ">","reference form with no valid local ref?!",off);
      printf(small_buf, refres, referr);
    }

    refres = dwarf_whatattr(attrib, &attr, &referr);
    if (refres != DW_DLV_OK) {
      snprintf(small_buf,sizeof(small_buf),
        "Form %d, has no attribute value?!" ,theform);
      printf(small_buf, refres, referr);
    }

    /*  Convert the local offset 'off' into a global section
      offset 'goff'. */
    refres = dwarf_convert_to_global_offset(attrib,
      off, &goff, &referr);
    if (refres != DW_DLV_OK) {
      /*  Report incorrect offset */
      snprintf(small_buf,sizeof(small_buf),
        "%s, GOFF=<0x%"  DW_PR_XZEROS  DW_PR_DUx
        ">","invalid offset",goff);
      printf(small_buf, refres, referr);
    }
    if (attr == DW_AT_sibling) {
      /*  The value had better be inside the current CU
        else there is a nasty error here, as a sibling
        has to be in the same CU, it seems. */
      /*  The target offset (off) had better be
        following the die's global offset else
        we have a serious botch. this FORM
        defines the value as a .debug_info
        global offset. */
      Dwarf_Off die_overall_offset = 0;
      int ores = dwarf_dieoffset(die, &die_overall_offset, &referr);
      if (ores != DW_DLV_OK) {
        printf("dwarf_dieoffset ores: %d err: %d", ores, referr);
      }
#ifdef NOTDEF
      SET_DIE_STACK_SIBLING(goff);
      if (die_overall_offset >= goff) {
        snprintf(small_buf,sizeof(small_buf),
          "ERROR: Sibling offset 0x%"  DW_PR_XZEROS  DW_PR_DUx
          " points %s its own die GOFF="
          "0x%"  DW_PR_XZEROS  DW_PR_DUx,
          goff,
          (die_overall_offset == goff)?"at":"before",
          die_overall_offset);
        printf(small_buf,DW_DLV_OK,0);
      }
#endif
    }

    /*  Do references inside <> to distinguish them ** from
      constants. In dense form this results in <<>>. Ugly for
      dense form, but better than ambiguous. davea 9/94 */
#ifdef NOTDEF
      bracket_hex("<",off,"",esbp);
      bracket_hex(" GOFF=",goff,">",esbp);
#endif

#ifdef NOTDEF
    if (check_type_offset) {
      if (attr == DW_AT_type && form_refers_local_info(theform)) {
        dres = dwarf_offdie_b(dbg, goff,
          is_info,
          &die_for_check, &referr);
        if (dres != DW_DLV_OK) {
          snprintf(small_buf,sizeof(small_buf),
            "DW_AT_type offset does not point to a DIE "
            "for global offset 0x%" DW_PR_XZEROS DW_PR_DUx
            " cu off 0x%" DW_PR_XZEROS DW_PR_DUx
            " local offset 0x%" DW_PR_XZEROS DW_PR_DUx
            " tag 0x%x",
            goff,dieprintCuGoffset,off,tag);
          DWARF_CHECK_ERROR(type_offset_result,small_buf);
        } else {
          int tres2 =
            dwarf_tag(die_for_check, &tag_for_check, &err);
          if (tres2 == DW_DLV_OK) {
            switch (tag_for_check) {
            case DW_TAG_array_type:
            case DW_TAG_class_type:
            case DW_TAG_enumeration_type:
            case DW_TAG_pointer_type:
            case DW_TAG_reference_type:
            case DW_TAG_rvalue_reference_type:
            case DW_TAG_restrict_type:
            case DW_TAG_string_type:
            case DW_TAG_structure_type:
            case DW_TAG_subroutine_type:
            case DW_TAG_typedef:
            case DW_TAG_union_type:
            case DW_TAG_ptr_to_member_type:
            case DW_TAG_set_type:
            case DW_TAG_subrange_type:
            case DW_TAG_base_type:
            case DW_TAG_const_type:
            case DW_TAG_file_type:
            case DW_TAG_packed_type:
            case DW_TAG_thrown_type:
            case DW_TAG_volatile_type:
            case DW_TAG_template_type_parameter:
            case DW_TAG_template_value_parameter:
            case DW_TAG_unspecified_type:
            /* Template alias */
            case DW_TAG_template_alias:
              /* OK */
              break;
            default:
              {
                snprintf(small_buf,sizeof(small_buf),
                  "DW_AT_type offset "
                  "0x%" DW_PR_XZEROS DW_PR_DUx
                  " does not point to Type"
                  " info we got tag 0x%x %s",
                (Dwarf_Unsigned)goff,
                tag_for_check,
                get_TAG_name(tag_for_check,
                  pd_dwarf_names_print_on_error));
                DWARF_CHECK_ERROR(type_offset_result,small_buf);
              }
              break;
            }
            dwarf_dealloc(dbg, die_for_check, DW_DLA_DIE);
            die_for_check = 0;
          } else {
            DWARF_CHECK_ERROR(type_offset_result,
              "DW_AT_type offset does not exist");
          }
        }
      }
    }
#endif
    }
    break;
  case DW_FORM_block:
  case DW_FORM_block1:
  case DW_FORM_block2:
  case DW_FORM_block4:
    fres = dwarf_formblock(attrib, &tempb, &err);
    if (fres == DW_DLV_OK) {
      unsigned u = 0;

      for (u = 0; u < tempb->bl_len; u++) {
        snprintf(small_buf, sizeof(small_buf), "%02x",
          *(u + (unsigned char *) tempb->bl_data));
        self->dwarfDbgEsb->esbAppend(self, esbp, small_buf);
      }
      dwarf_dealloc(dbg, tempb, DW_DLA_BLOCK);
      tempb = 0;
    } else {
      printf("DW_FORM_blockn cannot get block fres: %d err: %d\n", fres, err);
    }
    break;
  case DW_FORM_data1:
  case DW_FORM_data2:
  case DW_FORM_data4:
  case DW_FORM_data8:
    {
    Dwarf_Half attr = 0;
    fres = dwarf_whatattr(attrib, &attr, &err);
    if (fres == DW_DLV_ERROR) {
      printf("FORM_datan cannot get attr fres: %d err: %d", fres, err);
    } else if (fres == DW_DLV_NO_ENTRY) {
      printf("FORM_datan cannot get attr fres: %d err: %d", fres, err);
    } else {
      switch (attr) {
      case DW_AT_ordering:
      case DW_AT_byte_size:
      case DW_AT_bit_offset:
      case DW_AT_bit_size:
      case DW_AT_inline:
      case DW_AT_language:
      case DW_AT_visibility:
      case DW_AT_virtuality:
      case DW_AT_accessibility:
      case DW_AT_address_class:
      case DW_AT_calling_convention:
      case DW_AT_discr_list:    /* DWARF2 */
      case DW_AT_encoding:
      case DW_AT_identifier_case:
      case DW_AT_MIPS_loop_unroll_factor:
      case DW_AT_MIPS_software_pipeline_depth:
      case DW_AT_decl_column:
      case DW_AT_decl_file:
      case DW_AT_decl_line:
      case DW_AT_call_column:
      case DW_AT_call_file:
      case DW_AT_call_line:
      case DW_AT_start_scope:
      case DW_AT_byte_stride:
      case DW_AT_bit_stride:
      case DW_AT_count:
      case DW_AT_stmt_list:
      case DW_AT_MIPS_fde:
        {  int show_form_here = 0;
#ifdef NOTDEF
        wres = get_small_encoding_integer_and_name(dbg,
          attrib,
          &tempud,
          /* attrname */ (const char *) NULL,
          /* err_string */ ( dwarfDbgEsb_t *) NULL,
//          (encoding_type_func) 0,
          0,
          &err,show_form_here);
#endif
wres = 0;
        if (wres == DW_DLV_OK) {
          Dwarf_Bool hex_format = TRUE;
#ifdef NOTDEF
          formx_unsigned(tempud,esbp,hex_format);
#endif
          /* Check attribute encoding */
//          if (check_attr_encoding) {
//            check_attributes_encoding(attr,theform,tempud);
//          }
          if (attr == DW_AT_decl_file || attr == DW_AT_call_file) {
            if (srcfiles && tempud > 0 &&
              /* ASSERT: cnt >= 0 */
              tempud <= (Dwarf_Unsigned)cnt) {
              /*  added by user request */
              /*  srcfiles is indexed starting at 0, but
                DW_AT_decl_file defines that 0 means no
                file, so tempud 1 means the 0th entry in
                srcfiles, thus tempud-1 is the correct
                index into srcfiles.  */
              char *fname = srcfiles[tempud - 1];

              self->dwarfDbgEsb->esbAppend(self, esbp, " ");
              self->dwarfDbgEsb->esbAppend(self, esbp, fname);
            }

            /*  Validate integrity of files
              referenced in .debug_line */
#ifdef NOTDEF
            if (check_decl_file) {
              DWARF_CHECK_COUNT(decl_file_result,1);
              /*  Zero is always a legal index, it means
                no source name provided. */
              if (tempud != 0 &&
                tempud > ((Dwarf_Unsigned)cnt)) {
                if (!srcfiles) {
                  snprintf(small_buf,sizeof(small_buf),
                    "There is a file number=%" DW_PR_DUu
                    " but no source files "
                    " are known.",tempud);
                } else {
                  snprintf(small_buf, sizeof(small_buf),
                    "Does not point to valid file info "
                    " filenum=%"  DW_PR_DUu
                    " filecount=%" DW_PR_DUu ".",
                    tempud,cnt);
                }
                DWARF_CHECK_ERROR2(decl_file_result,
                  get_AT_name(attr,
                    pd_dwarf_names_print_on_error),
                  small_buf);
              }
            }
#endif
          }
        } else {
          printf("Cannot get encoding attribute .. wres: %d err: %d", wres, err);
        }
        }
        break;
      case DW_AT_const_value:
        /* Do not use hexadecimal format */
#ifdef NOTDEF
        wres = formxdata_print_value(dbg,die,attrib,esbp, &err, FALSE);
#endif
wres = 0;
        if (wres == DW_DLV_OK){
          /* String appended already. */
        } else if (wres == DW_DLV_NO_ENTRY) {
          /* nothing? */
        } else {
          printf("Cannot get DW_AT_const_value wres: %d err: %d",wres,err);
        }
        break;
      case DW_AT_GNU_dwo_id:
      case DW_AT_dwo_id:
        {
        Dwarf_Sig8 v;
        memset(&v,0,sizeof(v));
        wres = dwarf_formsig8_const(attrib,&v,&err);
        if (wres == DW_DLV_OK){
          dwarfDbgEsb_t t;

          self->dwarfDbgEsb->esbConstructor(self, &t);
#ifdef NOTDEF
          format_sig8_string(&v,&t);
#endif
          self->dwarfDbgEsb->esbAppend(self, esbp,self->dwarfDbgEsb->esbGetString(self, &t));
          self->dwarfDbgEsb->esbDestructor(self, &t);
        } else if (wres == DW_DLV_NO_ENTRY) {
          /* nothing? */
          self->dwarfDbgEsb->esbAppend(self, esbp,"Impossible: no entry for formsig8 dwo_id");
        } else {
          printf("Cannot get DW_AT_const_value wres: %d err: %d",wres,err);
        }
        }
        break;
      case DW_AT_upper_bound:
      case DW_AT_lower_bound:
      default:  {
        Dwarf_Bool chex = FALSE;
        Dwarf_Die  tdie = die;
        if(DW_AT_ranges == attr) {
          /*  In this case do not look for data
            type for unsigned/signed.
            and do use HEX. */
          chex = TRUE;
          tdie = NULL;
        }
        /* Do not use hexadecimal format except for
          DW_AT_ranges. */
#ifdef NOTDEF
        wres = formxdata_print_value(dbg,
          tdie,attrib,esbp, &err, chex);
#endif
wres = 0;
        if (wres == DW_DLV_OK) {
          /* String appended already. */
        } else if (wres == DW_DLV_NO_ENTRY) {
          /* nothing? */
        } else {
          printf("Cannot get form data.. wres: %d err: %d", wres, err);
        }
        }
        break;
      }
    }
#ifdef NOTDEF
    if (cu_name_flag) {
      if (attr == DW_AT_MIPS_fde) {
        if (fde_offset_for_cu_low == DW_DLV_BADOFFSET) {
          fde_offset_for_cu_low
            = fde_offset_for_cu_high = tempud;
        } else if (tempud < fde_offset_for_cu_low) {
          fde_offset_for_cu_low = tempud;
        } else if (tempud > fde_offset_for_cu_high) {
          fde_offset_for_cu_high = tempud;
        }
      }
    }
#endif
    }
    break;
  case DW_FORM_sdata:
    wres = dwarf_formsdata(attrib, &tempsd, &err);
    if (wres == DW_DLV_OK) {
      Dwarf_Bool hxform=TRUE;
      tempud = tempsd;
#ifdef NOTDEF
      formx_unsigned_and_signed_if_neg(tempud,tempsd,
        " (",hxform,esbp);
#endif
    } else if (wres == DW_DLV_NO_ENTRY) {
      /* nothing? */
    } else {
      printf("Cannot get formsdata..wres: %d err: %d", wres, err);
    }
    break;
  case DW_FORM_udata:
    wres = dwarf_formudata(attrib, &tempud, &err);
    if (wres == DW_DLV_OK) {
      Dwarf_Bool hex_format = TRUE;
#ifdef NOTDEF
      formx_unsigned(tempud,esbp,hex_format);
#endif
    } else if (wres == DW_DLV_NO_ENTRY) {
      /* nothing? */
    } else {
      printf("Cannot get formudata.... wres: %d err: %d", wres, err);
    }
    break;
  case DW_FORM_string:
  case DW_FORM_strp:
  case DW_FORM_strx:
  case DW_FORM_strp_sup: /* An offset to alternate file: tied file */
  case DW_FORM_GNU_strp_alt: /* An offset to alternate file: tied file */
  case DW_FORM_GNU_str_index: {
    int sres = dwarf_formstring(attrib, &temps, &err);
    if (sres == DW_DLV_OK) {
      if (theform == DW_FORM_strx ||
        theform == DW_FORM_GNU_str_index) {
        dwarfDbgEsb_t saver;
        Dwarf_Unsigned index = 0;

        self->dwarfDbgEsb->esbConstructor(self, &saver);
        sres = dwarf_get_debug_str_index(attrib,&index,&err);
        self->dwarfDbgEsb->esbAppend(self, &saver,temps);
        if(sres == DW_DLV_OK) {
#ifdef NOTDEF
          bracket_hex("(indexed string: ",index,")",esbp);
#endif
        } else {
          self->dwarfDbgEsb->esbAppend(self, esbp,"(indexed string:no string provided?)");
        }
        self->dwarfDbgEsb->esbAppend(self, esbp, self->dwarfDbgEsb->esbGetString(self, &saver));
        self->dwarfDbgEsb->esbDestructor(self, &saver);
      } else {
        self->dwarfDbgEsb->esbAppend(self, esbp,temps);
      }
    } else if (sres == DW_DLV_NO_ENTRY) {
      if (theform == DW_FORM_strx ||
        theform == DW_FORM_GNU_str_index) {
        self->dwarfDbgEsb->esbAppend(self, esbp, "(indexed string,no string provided?)");
      } else {
        self->dwarfDbgEsb->esbAppend(self, esbp, "<no string provided?>");
      }
    } else {
      if (theform == DW_FORM_strx ||
        theform == DW_FORM_GNU_str_index) {
        printf("Cannot get an indexed string.... sres: %d err: %d", sres, err);
      } else {
        printf("Cannot get a formstr (or a formstrp)....sres: %d err: %d", sres, err);
      }
    }
    }
    break;
  case DW_FORM_flag:
    wres = dwarf_formflag(attrib, &tempbool, &err);
    if (wres == DW_DLV_OK) {
      if (tempbool) {
        snprintf(small_buf, sizeof(small_buf), "yes(%d)",
          tempbool);
        self->dwarfDbgEsb->esbAppend(self, esbp, small_buf);
      } else {
        snprintf(small_buf, sizeof(small_buf), "no");
        self->dwarfDbgEsb->esbAppend(self, esbp, small_buf);
      }
    } else if (wres == DW_DLV_NO_ENTRY) {
      /* nothing? */
    } else {
      printf("Cannot get formflag/p....wres: %d err: %d", wres, err);
    }
    break;
  case DW_FORM_indirect:
    /*  We should not ever get here, since the true form was
      determined and direct_form has the DW_FORM_indirect if it is
      used here in this attr. */
#ifdef NOTDEF
    self->dwarfDbgEsb->esbAppend(self, esbp, get_FORM_name(theform,
      pd_dwarf_names_print_on_error));
#endif
    break;
  case DW_FORM_exprloc: {  /* DWARF4 */
    int showhextoo = 1;
#ifdef NOTDEF
    print_exprloc_content(dbg,die,attrib,showhextoo,esbp);
#endif
    }
    break;
  case DW_FORM_sec_offset: { /* DWARF4 */
    char* emptyattrname = 0;
    int show_form_here = 0;
#ifdef NOTDEF
    wres = get_small_encoding_integer_and_name(dbg,
      attrib,
      &tempud,
      emptyattrname,
      /* err_string */ NULL,
//      (encoding_type_func) 0,
      0,
      &err,show_form_here);
#endif
wres = 0;
    if (wres == DW_DLV_NO_ENTRY) {
      /* Show nothing? */
    } else if (wres == DW_DLV_ERROR) {
      printf("Cannot get a  DW_FORM_sec_offset....wres: %d err: %d", wres, err);
    } else {
#ifdef NOTDEF
      bracket_hex("",tempud,"",esbp);
#endif
    }
    }

    break;
  case DW_FORM_flag_present: /* DWARF4 */
    self->dwarfDbgEsb->esbAppend(self, esbp,"yes(1)");
    break;
  case DW_FORM_ref_sig8: {  /* DWARF4 */
    Dwarf_Sig8 sig8data;
    wres = dwarf_formsig8(attrib,&sig8data,&err);
    if (wres != DW_DLV_OK) {
      /* Show nothing? */
      printf("Cannot get a  DW_FORM_ref_sig8 ....wres: %d err: %d", wres, err);
    } else {
      dwarfDbgEsb_t sig8str;

      self->dwarfDbgEsb->esbConstructor(self, &sig8str);
#ifdef NOTDEF
      format_sig8_string(&sig8data,&sig8str);
#endif
      self->dwarfDbgEsb->esbAppend(self, esbp, self->dwarfDbgEsb->esbGetString(self, &sig8str));
      self->dwarfDbgEsb->esbDestructor(self, &sig8str);
      if (!show_form) {
        self->dwarfDbgEsb->esbAppend(self, esbp," <type signature>");
      }
    }
    }
    break;
  case DW_FORM_GNU_ref_alt: {
    bres = dwarf_global_formref(attrib, &off, &err);
    if (bres == DW_DLV_OK) {
#ifdef NOTDEF
      bracket_hex("",off,"",esbp);
#endif
    } else {
      printf("DW_FORM_GNU_ref_alt form with no reference?! bres: %d err: %d", bres, err);
    }
    }
    break;
  default:
    printf("dwarf_whatform unexpected value res: %d err: %d", DW_DLV_OK, err);
  }
#ifdef NOTDEF
  show_form_itself(show_form,local_verbose,theform, direct_form,esbp);
#endif
}

// =================================== get_producer_name =========================== 

/*  Returns the producer of the CU
  Caller must ensure producernameout is
  a valid, constructed, empty dwarfDbgEsb_t instance before calling.
  Never returns DW_DLV_ERROR.  */
static int getProducerName(dwarfDbgPtr_t self, Dwarf_Debug dbg, Dwarf_Die cuDie, Dwarf_Off dieprintCuOffset, dwarfDbgEsb_t *producernameout) {
  Dwarf_Attribute producer_attr = 0;
  Dwarf_Error pnerr = 0;

  int ares = dwarf_attr(cuDie, DW_AT_producer, &producer_attr, &pnerr);
  if (ares == DW_DLV_ERROR) {
    printf("hassattr on DW_AT_producer ares: %d pnerr: %d", ares, pnerr);
  }
  if (ares == DW_DLV_NO_ENTRY) {
    /*  We add extra quotes so it looks more like
      the names for real producers that get_attr_value
      produces. */
    self->dwarfDbgEsb->esbAppend(self, producernameout,"\"<CU-missing-DW_AT_producer>\"");
  } else {
    /*  DW_DLV_OK */
    /*  The string return is valid until the next call to this
      function; so if the caller needs to keep the returned
      string, the string must be copied (makename()). */
    get_attr_value(dbg, DW_TAG_compile_unit,
      cuDie, dieprintCuOffset,
      producer_attr, NULL, 0, producernameout,
      0 /*show_form_used*/,0 /* verbose */);
  }
  /*  If ares is error or missing case,
    producer_attr will be left
    NULL by the call,
    which is safe when calling dealloc(). */
  dwarf_dealloc(dbg, producer_attr, DW_DLA_ATTR);
  return ares;
}

// =================================== getCuName =========================== 

/* Returns the cu of the CU. In case of error, give up, do not return. */
static int getCuName(dwarfDbgPtr_t self, Dwarf_Debug dbg, Dwarf_Die cuDie, Dwarf_Off dieprintCuOffset, char * *short_name, char * *long_name) {
  Dwarf_Attribute name_attr = 0;
  Dwarf_Error lerr = 0;
  int ares;

  ares = dwarf_attr(cuDie, DW_AT_name, &name_attr, &lerr);
  if (ares == DW_DLV_ERROR) {
    printf("hassattr on DW_AT_name ares: %d lerr: %d", ares, lerr);
  } else {
    if (ares == DW_DLV_NO_ENTRY) {
      *short_name = "<unknown name>";
      *long_name = "<unknown name>";
    } else {
      /* DW_DLV_OK */
      /*  The string return is valid until the next call to this
        function; so if the caller needs to keep the returned
        string, the string must be copied (makename()). */
      char *filename = 0;

      self->dwarfDbgEsb->esbEmptyString(self, &esbLongCuName);
      get_attr_value(dbg, DW_TAG_compile_unit,
        cuDie, dieprintCuOffset,
        name_attr, NULL, 0, &esbLongCuName,
        0 /*show_form_used*/,0 /* verbose */);
      *long_name = self->dwarfDbgEsb->esbGetString(self, &esbLongCuName);
      /* Generate the short name (filename) */
      filename = strrchr(*long_name,'/');
      if (!filename) {
        filename = strrchr(*long_name,'\\');
      }
      if (filename) {
        ++filename;
      } else {
        filename = *long_name;
      }
      self->dwarfDbgEsb->esbEmptyString(self, &esbShortCuName);
      self->dwarfDbgEsb->esbAppend(self, &esbShortCuName,filename);
      *short_name = self->dwarfDbgEsb->esbGetString(self, &esbShortCuName);
    }
  }
  dwarf_dealloc(dbg, name_attr, DW_DLA_ATTR);
  return ares;
}

// =================================== handle_one_die_section =========================== 

static int handle_one_die_section(dwarfDbgPtr_t self, Dwarf_Debug dbg, Dwarf_Bool is_info, Dwarf_Error *pod_err) {
  Dwarf_Unsigned cuHeaderLength = 0;
  Dwarf_Unsigned abbrev_offset = 0;
  Dwarf_Half version_stamp = 0;
  Dwarf_Half address_size = 0;
  Dwarf_Half extension_size = 0;
  Dwarf_Half length_size = 0;
  Dwarf_Sig8 signature;
  Dwarf_Unsigned typeoffset = 0;
  Dwarf_Unsigned nextCuOffset = 0;
  unsigned loop_count = 0;
  int nres = DW_DLV_OK;
  int   cuCount = 0;
  char * cuShortName = NULL;
  char * cuLongName = NULL;
  const char * section_name = 0;
  int res = 0;
  Dwarf_Off dieprintCuGoffset = 0;

  /* Loop until it fails.  */
  for (;;++loop_count) {
    int sres = DW_DLV_OK;
    Dwarf_Die cuDie = 0;
    struct Dwarf_Debug_Fission_Per_CU_s fission_data;
    int fission_data_result = 0;
    Dwarf_Half cuType = 0;

    memset(&fission_data,0,sizeof(fission_data));
    nres = dwarf_next_cu_header_d(dbg, is_info, &cuHeaderLength, &version_stamp,
      &abbrev_offset, &address_size, &length_size,&extension_size, &signature, &typeoffset,
      &nextCuOffset, &cuType, pod_err);
    if (nres != DW_DLV_OK) {
      return nres;
    }
printf("nextCuOffset: %p cuHeaderLength: %d abbrev_offset: %d address_size: %d length_size: %d\n", nextCuOffset, cuHeaderLength, abbrev_offset, address_size, length_size);
fflush(stdout);
    /*  get basic information about the current CU: producer, name */
    sres = dwarf_siblingof_b(dbg, NULL,is_info, &cuDie, pod_err);
    if (sres != DW_DLV_OK) {
      dieprintCuGoffset = 0;
      printf("siblingof cu header sres: %d pod_err: %s", sres, *pod_err);
    }
    /* Get the CU offset for easy error reporting */
    dwarf_die_offsets(cuDie, &DIEOverallOffset, &DIEOffset,pod_err);
    DIECuOverallOffset = DIEOverallOffset;
    DIECuOffset = DIEOffset;
    dieprintCuGoffset = DIEOverallOffset;
printf("dieprintCuGoffset: %d\n", dieprintCuGoffset);
    {
    /* Get producer name for this CU and update compiler list */
      dwarfDbgEsb_t producername;

      self->dwarfDbgEsb->esbConstructor(self, &producername);
      self->dwarfDbgElfInfo->getProducerName(self, dbg, cuDie, dieprintCuGoffset, &producername);
//printf("producername: %s\n", self->dwarfDbgEsb->esbGetString(self, &producername));
#ifdef NOTDEF
      update_compiler_target(self->dwarfDbgEsb->esbGetString(self, &producername));
#endif
      self->dwarfDbgEsb->esbDestructor(self, &producername);
    }
    /*  Once the compiler table has been updated, see
      if we need to generate the list of CU compiled
      by all the producers contained in the elf file */
    self->dwarfDbgElfInfo->getCuName(self, dbg, cuDie, dieprintCuGoffset, &cuShortName, &cuLongName);
printf("cuShortName: %s\n", cuShortName);
    /* Add CU name to current compiler entry */
//    add_cu_name_compiler_target(cuLongName);
    /*  Some prerelease gcc versions used ranges but seemingly
      assumed the lack of a base address in the CU was
      defined to be a zero base.
      Assuming a base address (and low and high) is sensible. */
    CuBaseAddress = 0;
    CuHighAddress = 0;
    CuLowAddress = 0;

    /*  Release the 'cuDie' created by the call
      to 'dwarf_siblingof' at the top of the main loop. */
    dwarf_dealloc(dbg, cuDie, DW_DLA_DIE);
    cuDie = 0; /* For debugging, stale die should be NULL. */

    /*  Process a single compilation unit in .debug_info or
      .debug_types. */
    sres = dwarf_siblingof_b(dbg, NULL, is_info, &cuDie, pod_err);
    if (sres == DW_DLV_OK) {
        Dwarf_Signed cnt = 0;
        char **srcfiles = 0;
        Dwarf_Error srcerr = 0;
        int i = 0;
        int srcf = dwarf_srcfiles(cuDie, &srcfiles, &cnt, &srcerr);

        if (srcf == DW_DLV_ERROR) {
          printf("dwarf_srcfiles srcf: %d srcerr: %d", srcf,srcerr);
          dwarf_dealloc(dbg,srcerr,DW_DLA_ERROR);
          srcerr = 0;
          srcfiles = 0;
          cnt = 0;
        } /*DW_DLV_NO_ENTRY generally means there
          there is no DW_AT_stmt_list attribute.
          and we do not want to print anything
          about statements in that case */

        /* Get the CU offset for easy error reporting */
        dwarf_die_offsets(cuDie, &DIEOverallOffset, &DIEOffset, pod_err);
        DIECuOverallOffset = DIEOverallOffset;
        DIECuOffset = DIEOffset;
        dieprintCuGoffset = DIEOverallOffset;
printf("DIEOverallOffset: %d DIEOffset: %d \n", DIEOverallOffset, DIEOffset);
        if (srcf == DW_DLV_OK) {
          int si = 0;

          // handle source (*.c) and include files (*.h)
          for (si = 0; si < cnt; ++si) {
            printf("src: %d %s\n", si, srcfiles[si]);
            dwarf_dealloc(dbg, srcfiles[si], DW_DLA_STRING);
          }
          dwarf_dealloc(dbg, srcfiles, DW_DLA_LIST);
        }
    }

  }
}

// =================================== dwarfDbgOpenElf =========================== 

int dwarfDbgOpenElf (dwarfDbgPtr_t self, char *fileName) {
  int f = 0;
  Elf_Cmd cmd = 0;
  Elf *arf = 0;
  Elf *elf = 0;
  Dwarf_Debug dbg = 0;
  int dres = 0;
  Dwarf_Error onef_err = 0;
  Dwarf_Error pod_err;

printf("dwarfDbgOpenElf\n");
fflush(stdout);
  self->dwarfDbgEsb->esbConstructor(self, &config_file_path);
  self->dwarfDbgEsb->esbConstructor(self, &config_file_tiedpath);
  self->dwarfDbgEsb->esbConstructor(self, &esbShortCuName);
  self->dwarfDbgEsb->esbConstructor(self, &esbLongCuName);
  self->dwarfDbgEsb->esbConstructor(self, &dwarf_error_line);
  (void) elf_version(EV_NONE);
  if (elf_version(EV_CURRENT) == EV_NONE) {
  self->errorStr = "dwarfDbg: libelf.a out of date.";
  return TCL_ERROR;
  }
  f = open(fileName, O_RDONLY);
  cmd = ELF_C_READ;
printf("cmd1: %d\n", cmd);
  arf = elf_begin(f, cmd, (Elf *) 0);
printf("fileName: %s f: %d arf: %p\n", fileName, f, arf);
fflush(stdout);
  // we only handle one elf part in this version!!
  while ((elf = elf_begin(f, cmd, arf)) != 0) {
printf("elf: %p\n", elf);
  dres = dwarf_elf_init(elf, DW_DLC_READ, NULL, NULL, &dbg, &onef_err);
printf("dbg: %p dres: %d onef_err: %d\n", dbg, dres, onef_err);
  if (dres == DW_DLV_NO_ENTRY) {
    sprintf(self->errorBuf, "No DWARF information present in %s\n", fileName);
    self->errorStr = self->errorBuf;
    return TCL_ERROR;
  }   
  printf("dwarf_elf_int ok for %p\n", elf);
    handle_one_die_section(self, dbg,1, &pod_err);
  cmd = elf_next(elf);
printf("cmd2: %d\n", cmd);
  if (cmd != 0) {
    // we cannot handle more than one elf part right now!!
    self->errorStr = "too many elf parts";
    return TCL_ERROR;
   }
  elf_end(elf);
  }
  elf_end(arf);
  return TCL_OK;
}

// =================================== dwarfDbgElfInfoInit =========================== 

int dwarfDbgElfInfoInit (dwarfDbgPtr_t self) {
printf("dwarfDbgElfInfoInit\n");
  _self = self;
  self->dwarfDbgElfInfo->getProducerName = getProducerName;
  self->dwarfDbgElfInfo->getCuName = getCuName;
  return DWARF_DBG_ERR_OK;
}

