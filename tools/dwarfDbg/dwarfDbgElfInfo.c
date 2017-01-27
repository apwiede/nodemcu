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
 * File:   dwarfDbgElfInfo.c
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


/*  Encodings have undefined signedness. Accept either
    signedness.  The values are integer-like (they are defined
    in the DWARF specification), so the
    form the compiler uses (as long as it is
    a constant value) is a non-issue.

    The numbers need not be small (in spite of the
    function name), but the result should be an integer.

    If string_out is non-NULL, construct a string output, either
    an error message or the name of the encoding.
    The function pointer passed in is to code generated
    by a script at dwarfdump build time. The code for
    the val_as_string function is generated
    from dwarf.h.  See <build dir>/dwarf_names.c

    The known_signed bool is set true(nonzero) or false (zero)
    and *both* uval_out and sval_out are set to the value,
    though of course uval_out cannot represent a signed
    value properly and sval_out cannot represent all unsigned
    values properly.

    If string_out is non-NULL then attr_name and val_as_string
    must also be non-NULL.  */
static int
get_small_encoding_integer_and_name(Dwarf_Debug dbg,
    Dwarf_Attribute attrib,
    Dwarf_Unsigned * uval_out,
    const char *attr_name,
    dwarfDbgEsb_t * string_out,
//    encoding_type_func val_as_string,
    Dwarf_Error * seierr,
    int show_form)
{
    Dwarf_Unsigned uval = 0;
    char buf[100];              /* The strings are small. */
    int vres = dwarf_formudata(attrib, &uval, seierr);

printf("get_small_encoding_integer_and_name: uval: %d\n", uval);
    if (vres != DW_DLV_OK) {
        Dwarf_Signed sval = 0;
        if(vres == DW_DLV_ERROR) {
            dwarf_dealloc(dbg,*seierr, DW_DLV_ERROR);
            *seierr = 0;
        }
        vres = dwarf_formsdata(attrib, &sval, seierr);
        if (vres != DW_DLV_OK) {
            vres = dwarf_global_formref(attrib,&uval,seierr);
            if (vres != DW_DLV_OK) {
#ifdef NOTDEF
                if (string_out != 0) {
                    snprintf(buf, sizeof(buf),
                        "%s has a bad form.", attr_name);
                    esb_append(string_out,buf);
                }
#endif
                return vres;
            }
            *uval_out = uval;
        } else {
            uval =  (Dwarf_Unsigned) sval;
            *uval_out = uval;
        }
    } else {
        *uval_out = uval;
    }
#ifdef NOTDEF
    if (string_out) {
        Dwarf_Half theform = 0;
        Dwarf_Half directform = 0;
        struct esb_s fstring;

        esb_constructor(&fstring);
        get_form_values(dbg,attrib,&theform,&directform);
        esb_append(&fstring, val_as_string((Dwarf_Half) uval,
            pd_dwarf_names_print_on_error));
        show_form_itself(show_form, verbose, theform, directform,&fstring);
        esb_append(string_out,esb_get_string(&fstring));
        esb_destructor(&fstring);
    }
#endif
    return DW_DLV_OK;
}

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

printf("get_attr_value: theform: 0x%08x attrib: 0x%08x\n", theform, attrib);
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
#ifdef NOTDEF
          DWARF_CHECK_ERROR(type_offset_result,small_buf);
#endif
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
#ifdef NOTDEF
                DWARF_CHECK_ERROR(type_offset_result,small_buf);
#endif
              }
              break;
            }
            dwarf_dealloc(dbg, die_for_check, DW_DLA_DIE);
            die_for_check = 0;
          } else {
#ifdef NOTDEF
            DWARF_CHECK_ERROR(type_offset_result,
              "DW_AT_type offset does not exist");
#endif
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
        wres = get_small_encoding_integer_and_name(dbg,
          attrib,
          &tempud,
          /* attrname */ (const char *) NULL,
          /* err_string */ ( dwarfDbgEsb_t *) NULL,
//          (encoding_type_func) 0,
          &err,show_form_here);
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
#ifdef NOTDEF
                DWARF_CHECK_ERROR2(decl_file_result,
                  get_AT_name(attr,
                    pd_dwarf_names_print_on_error),
                  small_buf);
#endif
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
    wres = get_small_encoding_integer_and_name(dbg,
      attrib,
      &tempud,
      emptyattrname,
      /* err_string */ NULL,
//      (encoding_type_func) 0,
      &err,show_form_here);
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


static int die_stack_indent_level = 0;
static int local_symbols_already_began = FALSE;
int stop_indent_level = 0;
typedef enum /* Dwarf_Check_Categories */ {
    abbrev_code_result,
    pubname_attr_result,
    reloc_offset_result,
    attr_tag_result,
    tag_tree_result,
    type_offset_result,
    decl_file_result,
    ranges_result,
    lines_result,
    aranges_result,
    /*  Harmless errors are errors detected inside libdwarf but
        not reported via DW_DLE_ERROR returns because the errors
        won't really affect client code.  The 'harmless' errors
        are reported and otherwise ignored.  It is difficult to report
        the error when the error is noticed by libdwarf, the error
        is reported at a later time.
        The other errors dwarfdump reports are also generally harmless
        but are detected by dwarfdump so it's possble to report the
        error as soon as the error is discovered. */
    harmless_result,
    fde_duplication,
    frames_result,
    locations_result,
    names_result,
    abbreviations_result,
    dwarf_constants_result,
    di_gaps_result,
    forward_decl_result,
    self_references_result,
    attr_encoding_result,
    duplicated_attributes_result,
    total_check_result,
    LAST_CATEGORY  /* Must be last */
} Dwarf_Check_Categories;

print_die_stack(Dwarf_Debug dbg, char **srcfiles, Dwarf_Signed cnt) {
//printf("print_die_stack called\n");
}
static int pd_dwarf_names_print_on_error = 1;
static Dwarf_Bool bSawLow = FALSE;
static Dwarf_Bool bSawHigh = FALSE;


const char * get_TAG_name(unsigned int val_in,int printonerr)
{
   const char *v = 0;
   int res = dwarf_get_TAG_name(val_in,&v);
printf("get_TAG_name: %d %s\n", val_in, v);
   return v;
//   return ellipname(res,val_in,v,"TAG",printonerr);
}
const char * get_AT_name(unsigned int val_in,int printonerr)
{
   const char *v = 0;
   int res = dwarf_get_AT_name(val_in,&v);
printf("get_AT_name: %d %s\n", val_in, v);
   return v;
//   return ellipname(res,val_in,v,"AT",printonerr);
}

// =================================== getCompileUnitLineInfos =========================== 

static uint8_t getCompileUnitLineInfos(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Off dieprint_cu_goffset) {
    Dwarf_Unsigned lineversion = 0;
    Dwarf_Signed linecount = 0;
    Dwarf_Line *linebuf = NULL;
    Dwarf_Signed linecount_actuals = 0;
    Dwarf_Line *linebuf_actuals = NULL;
    Dwarf_Small  table_count = 0;
    int lres = 0;
    int line_errs = 0;
    Dwarf_Line_Context line_context = 0;
    const char *sec_name = 0;
    Dwarf_Error err = 0;
    Dwarf_Off cudie_local_offset = 0;
//    Dwarf_Off dieprint_cu_goffset = 0;
    int lires = 0;
    int atres = 0;
    int ares = 0;
    Dwarf_Unsigned lineno = 0;

printf("getCompileUnitLineInfos\n");
        lres = dwarf_srclines_b(die,&lineversion,
            &table_count,&line_context,
            &err);
if (table_count > 0) {
printf(">>table_count: %d\n", table_count);
}
        if(lres == DW_DLV_OK) {
printf("dwarf_srclines_two_level_from_linecontext\n");
            lres = dwarf_srclines_two_level_from_linecontext(line_context,
                &linebuf, &linecount,
                &linebuf_actuals, &linecount_actuals,
                &err);
        }
if (linecount > 0) {
printf(">>>linecount: %d\n", linecount);

    Dwarf_Signed i = 0;
    Dwarf_Addr pc = 0;
    Dwarf_Error lt_err = 0;

    for (i = 0; i < linecount; i++) {
        Dwarf_Line line = linebuf[i];
        char* filename = 0;
        int nsres = 0;

        pc = 0;
        ares = dwarf_lineaddr(line, &pc, &lt_err);
        lires = dwarf_lineno(line, &lineno, &lt_err);
printf("dwarf_lineaddr: line: 0x%08x pc: 0x%08x lineno: %d\n", line, pc, lineno);
    }
}
   return 1;
}


// =================================== print_attribute =========================== 

static int
print_attribute(Dwarf_Debug dbg, Dwarf_Die die,
    Dwarf_Off dieprint_cu_goffset,
    Dwarf_Half attr,
    Dwarf_Attribute attr_in,
    int print_information,
    int die_indent_level,
    char **srcfiles, Dwarf_Signed cnt)
{
    Dwarf_Attribute attrib = 0;
    Dwarf_Unsigned uval = 0;
    const char * atname = 0;
    dwarfDbgEsb_t valname;
    dwarfDbgEsb_t esb_extra;
    int tres = 0;
    Dwarf_Half tag = 0;
    int append_extra_string = 0;
    int found_search_attr = FALSE;
    int bTextFound = FALSE;
    Dwarf_Bool is_info = FALSE;
    Dwarf_Addr elf_max_address = 0;
    Dwarf_Error paerr = 0;

    char * temps = NULL;
    int fres = 0;
    int sres = 0;
    int bres = 0;
    Dwarf_Half theform = 0;
    Dwarf_Error err = 0;
    Dwarf_Addr addr = 0;
    
//printf("print_attribute attr: 0x%08x\n", attr_in);
//    esb_constructor(&esb_extra);
//    esb_constructor(&valname);
//    is_info = dwarf_get_die_infotypes_flag(die);
    atname = get_AT_name(attr,pd_dwarf_names_print_on_error);

    tres = dwarf_tag(die, &tag, &err);
    fres = dwarf_whatform(attr_in, &theform, &err);
printf("get_attr_value: tag: %s attr: 0x%08x theform: 0x%08x\n", get_TAG_name(tag, 0), attr_in, theform);

    switch (theform) {
    case DW_FORM_addr:
      bres = dwarf_formaddr(attr_in, &addr, &err);
printf("DW_FORM_addr: attrib: 0x%08x addr: 0x%08x\n", attr_in, addr);
      break;
    case DW_FORM_data1:
      fres = dwarf_whatattr(attr_in, &attr, &err);
printf("DW_FORM_data1 attr: 0x%08x\n", attr);
      switch(attr) {
      case DW_AT_language:
printf("DW_AT_language\n");
        break;
      case DW_AT_byte_size:
printf("DW_AT_byte_size\n");
        break;
      case DW_AT_encoding:
printf("DW_AT_encoding\n");
        break;
      case DW_AT_decl_file:
printf("DW_AT_decl_file\n");
        break;
      case DW_AT_decl_line:
printf("DW_AT_decl_line\n");
        break;
      case DW_AT_upper_bound:
printf("DW_AT_upper_bound\n");
        break;
      case DW_AT_data_member_location:
printf("DW_AT_data_member_location\n");
        break;
      case DW_AT_inline:
printf("DW_AT_inline\n");
        break;
      case DW_AT_const_value:
printf("DW_AT_const_value\n");
        break;
      case DW_AT_call_file:
printf("DW_AT_call_file\n");
        break;
      case DW_AT_call_line:
printf("DW_AT_call_line\n");
        break;
      case DW_AT_bit_offset:
printf("DW_AT_bit_offset\n");
        break;
      case DW_AT_bit_size:
printf("DW_AT_bit_size\n");
        break;
      default:
printf("ERROR attribute: 0x%08x not yet implemented\n", attr);
        break;
      }
      break;
    case DW_FORM_strp:
        sres = dwarf_formstring(attr_in, &temps, &err);
printf("DW_FORM_strp: attrib: 0x%08x temps: %s\n", attr_in, temps);
      break;
    case DW_FORM_sec_offset:
printf("DW_FORM_sec_offset\n");
      break;
    case DW_FORM_ref4:
printf("DW_FORM_ref4\n");
      break;
printf("ERROR theform: 0x&08x not yet implemented\n", theform);
    default:
      break;
    }
    switch (attr) {
    case DW_AT_language:
        get_small_encoding_integer_and_name(dbg, attrib, &uval,
            "DW_AT_language", &valname,
//            get_LANG_name,
            &paerr,
            1);
        break;
     }

   return 1;
}

// =================================== print_one_die =========================== 

int print_one_die(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Off dieprint_cu_goffset, int print_information,
    int die_indent_level, char **srcfiles, Dwarf_Signed cnt, int ignore_die_stack) {
    Dwarf_Signed i = 0;
    Dwarf_Signed j = 0;
    Dwarf_Off offset = 0;
    Dwarf_Off overall_offset = 0;
    const char * tagname = 0;
    Dwarf_Half tag = 0;
    Dwarf_Signed atcnt = 0;
    Dwarf_Attribute *atlist = 0;
    int tres = 0;
    int ores = 0;
    int atres = 0;
    int abbrev_code = dwarf_die_abbrev_code(die);
    int attribute_matched = FALSE;
    Dwarf_Error podie_err = 0;
uint8_t result;

//printf("print_one_die\n");
result = getCompileUnitLineInfos(dbg, die, dieprint_cu_goffset);
    /* Attribute indent. */
    int nColumn = 18;

    tres = dwarf_tag(die, &tag, &podie_err);
    if (tres != DW_DLV_OK) {
        printf("accessing tag of die! tres: %d, podie_err: %d", tres, podie_err);
    }
    tagname = get_TAG_name(tag,pd_dwarf_names_print_on_error);

    ores = dwarf_die_CU_offset(die, &offset, &podie_err);
    if (ores != DW_DLV_OK) {
        printf("dwarf_die_CU_offset ores: %d podie_err: %d", ores, podie_err);
    }
printf("dwarf_die_cu_offset: offset: %d\n", offset);

    atres = dwarf_attrlist(die, &atlist, &atcnt, &podie_err);
    if (atres == DW_DLV_ERROR) {
        printf("dwarf_attrlist atres: %d podie_err: %d", atres, podie_err);
    } else if (atres == DW_DLV_NO_ENTRY) {
        /* indicates there are no attrs.  It is not an error. */
        atcnt = 0;
    }
printf("atcnt: %d\n", atcnt);

    /* Reset any loose references to low or high PC */
    bSawLow = FALSE;
    bSawHigh = FALSE;

    /* Get the offset for easy error reporting: This is not the CU die.  */
    dwarf_die_offsets(die,&DIEOverallOffset,&DIEOffset,&podie_err);

    for (i = 0; i < atcnt; i++) {
        Dwarf_Half attr;
        int ares;

        ares = dwarf_whatattr(atlist[i], &attr, &podie_err);

        if (ares == DW_DLV_OK) {
            /*  Check duplicated attributes; use brute force as the number of
                attributes is quite small; the problem was detected with the
                LLVM toolchain, generating more than 12 repeated attributes */
#ifdef NOTDEF
            if (check_duplicated_attributes) {
                Dwarf_Half attr_next;
                DWARF_CHECK_COUNT(duplicated_attributes_result,1);
                for (j = i + 1; j < atcnt; ++j) {
                    ares = dwarf_whatattr(atlist[j], &attr_next, &podie_err);
                    if (ares == DW_DLV_OK) {
                        if (attr == attr_next) {
                            DWARF_CHECK_ERROR2(duplicated_attributes_result,
                                "Duplicated attribute ",
                                get_AT_name(attr,pd_dwarf_names_print_on_error));
                        }
                    } else {
                        printf("dwarf_whatattr entry missing ares: %d podie_err: %d", ares, podie_err);
                    }
                }
            }

            /* Print using indentation */
            if (!dense && PRINTING_DIES && print_information) {
                printf("%*s",die_indent_level * 2 + 2 + nColumn," ");
            }
#endif

            {
//printf("call print_attribute\n");
                int attr_match = print_attribute(dbg, die,
                    dieprint_cu_goffset,
                    attr,
                    atlist[i],
                    print_information, die_indent_level, srcfiles, cnt);
#ifdef NOTDEF
                if (print_information == FALSE && attr_match) {
                    attribute_matched = TRUE;
                }
#endif
            }

#ifdef NOTDEF
            if (record_dwarf_error && check_verbose_mode) {
                record_dwarf_error = FALSE;
            }
#endif
        } else {
            printf("dwarf_whatattr entry missing ares: %d podie_err: %d", ares, podie_err);
        }
    }

    for (i = 0; i < atcnt; i++) {
        dwarf_dealloc(dbg, atlist[i], DW_DLA_ATTR);
    }
    if (atres == DW_DLV_OK) {
        dwarf_dealloc(dbg, atlist, DW_DLA_LIST);
    }

#ifdef NOTDEF
    if (PRINTING_DIES && dense && print_information) {
        printf("\n");
    }
#endif
    return attribute_matched;
}

// =================================== print_die_and_children_internal =========================== 

/* recursively follow the die tree */
static void print_die_and_children_internal(Dwarf_Debug dbg, Dwarf_Die in_die_in, Dwarf_Off dieprint_cu_goffset,
    Dwarf_Bool is_info, char **srcfiles, Dwarf_Signed cnt) {
    Dwarf_Die child = 0;
    Dwarf_Die sibling = 0;
    Dwarf_Error dacerr = 0;
    int tres = 0;
    int cdres = 0;
    Dwarf_Die in_die = in_die_in;

    for (;;) {
        /* Get the CU offset for easy error reporting */
        dwarf_die_offsets(in_die,&DIEOverallOffset,&DIEOffset,&dacerr);

printf("print_die_and_children_internal: DIE_overall_offset: %d DIE_offset: %d\n", DIEOverallOffset, DIEOffset);
#ifdef NOTDEF
        SET_DIE_STACK_ENTRY(die_stack_indent_level,in_die, dieprint_cu_goffset);

//        if (check_tag_tree || print_usage_tag_attr) {
            DWARF_CHECK_COUNT(tag_tree_result,1);
            if (die_stack_indent_level == 0) {
                Dwarf_Half tag = 0;

                tres = dwarf_tag(in_die, &tag, &dacerr);
                if (tres != DW_DLV_OK) {
#ifdef NOTDEF
                    DWARF_CHECK_ERROR(tag_tree_result,
                        "Tag-tree root tag unavailable: "
                        "is not DW_TAG_compile_unit");
#endif
                } else if (tag == DW_TAG_compile_unit) {
                    /* OK */
                } else if (tag == DW_TAG_partial_unit) {
                    /* OK */
                } else if (tag == DW_TAG_type_unit) {
                    /* OK */
                } else {
#ifdef NOTDEF
                    DWARF_CHECK_ERROR(tag_tree_result,
                        "tag-tree root is not DW_TAG_compile_unit "
                        "or DW_TAG_partial_unit or DW_TAG_type_unit");
#endif
                }
            } else {
                Dwarf_Half tag_parent = 0;
                Dwarf_Half tag_child = 0;
                Dwarf_Error dacerr2 = 0;
                int pres = 0;
                int cres = 0;
                const char *ctagname = "<child tag invalid>";
                const char *ptagname = "<parent tag invalid>";

                pres = dwarf_tag(die_stack[die_stack_indent_level - 1].die_,
                    &tag_parent, &dacerr);
                cres = dwarf_tag(in_die, &tag_child, &dacerr2);
                if (pres != DW_DLV_OK)
                    tag_parent = 0;
                if (cres != DW_DLV_OK)
                    tag_child = 0;
                DROP_ERROR_INSTANCE(dbg,pres,dacerr);
                DROP_ERROR_INSTANCE(dbg,cres,dacerr2);

#ifdef NOTDEF
                /* Check for specific compiler */
//                if (checking_this_compiler()) {
                    /* Process specific TAGs. */
                    tag_specific_checks_setup(tag_child,die_stack_indent_level);
                    if (cres != DW_DLV_OK || pres != DW_DLV_OK) {
                        if (cres == DW_DLV_OK) {
                            ctagname = get_TAG_name(tag_child,
                                pd_dwarf_names_print_on_error);
                        }
                        if (pres == DW_DLV_OK) {
                            ptagname = get_TAG_name(tag_parent,
                                pd_dwarf_names_print_on_error);
                        }
#ifdef NOTDEF
                        DWARF_CHECK_ERROR3(tag_tree_result,ptagname,
                            ctagname,
                            "Tag-tree relation is not standard..");
#endif
                    } else if (legal_tag_tree_combination(tag_parent,
                        tag_child)) {
                        /* OK */
                    } else {
                        /* Report errors only if tag-tree check is on */
                        if (check_tag_tree) {
#ifdef NOTDEF
                            DWARF_CHECK_ERROR3(tag_tree_result,
                                get_TAG_name(tag_parent,
                                    pd_dwarf_names_print_on_error),
                                get_TAG_name(tag_child,
                                    pd_dwarf_names_print_on_error),
                                "tag-tree relation is not standard.");
#endif
                        }
                    }
//                }
#endif
            }
//        }

        if (record_dwarf_error && check_verbose_mode) {
            record_dwarf_error = FALSE;
        }
#endif

        /* Here do pre-descent processing of the die. */
        {
            int retry_print_on_match = FALSE;
            int ignore_die_stack = FALSE;
            retry_print_on_match = print_one_die(dbg, in_die, dieprint_cu_goffset,
//                print_as_info_or_cu(), die_stack_indent_level, srcfiles, cnt,ignore_die_stack);
                0, die_stack_indent_level, srcfiles, cnt,ignore_die_stack);
#ifdef NOTDEF
            validate_die_stack_siblings(dbg);
#endif
                    print_die_stack(dbg,srcfiles,cnt);
                    stop_indent_level = die_stack_indent_level;
//                    info_flag = TRUE;
        }

        cdres = dwarf_child(in_die, &child, &dacerr);
        /* Check for specific compiler */
//        if (check_abbreviations && checking_this_compiler()) {
            Dwarf_Half ab_has_child;
            Dwarf_Bool bError = FALSE;
            Dwarf_Half tag = 0;
            tres = dwarf_die_abbrev_children_flag(in_die,&ab_has_child);
            if (tres == DW_DLV_OK) {
#ifdef NOTDEF
                DWARF_CHECK_COUNT(abbreviations_result,1);
#endif
                tres = dwarf_tag(in_die, &tag, &dacerr);
                if (tres == DW_DLV_OK) {
                    switch (tag) {
                    case DW_TAG_array_type:
                    case DW_TAG_class_type:
                    case DW_TAG_compile_unit:
                    case DW_TAG_type_unit:
                    case DW_TAG_partial_unit:
                    case DW_TAG_enumeration_type:
                    case DW_TAG_lexical_block:
                    case DW_TAG_namespace:
                    case DW_TAG_structure_type:
                    case DW_TAG_subprogram:
                    case DW_TAG_subroutine_type:
                    case DW_TAG_union_type:
                    case DW_TAG_entry_point:
                    case DW_TAG_inlined_subroutine:
                        break;
                    default:
                        bError = (cdres == DW_DLV_OK && !ab_has_child) ||
                            (cdres == DW_DLV_NO_ENTRY && ab_has_child);
                        if (bError) {
#ifdef NOTDEF
                            DWARF_CHECK_ERROR(abbreviations_result,
                                "check 'dw_children' flag combination.");
#endif
                        }
                        break;
                    }
                }
            }
//        }


        /* child first: we are doing depth-first walk */
        if (cdres == DW_DLV_OK) {
            /*  If the global offset of the (first) child is
                <= the parent DW_AT_sibling global-offset-value
                then the compiler has made a mistake, and
                the DIE tree is corrupt.  */
            Dwarf_Off child_overall_offset = 0;
            int cores = dwarf_dieoffset(child, &child_overall_offset, &dacerr);
            if (cores == DW_DLV_OK) {
                char small_buf[200];
#ifdef NOTDEF
                Dwarf_Off parent_sib_val = get_die_stack_sibling();
                if (parent_sib_val &&
                    (parent_sib_val <= child_overall_offset )) {
                    snprintf(small_buf,sizeof(small_buf),
                        "A parent DW_AT_sibling of "
                        "0x%" DW_PR_XZEROS  DW_PR_DUx
                        " points %s the first child "
                        "0x%"  DW_PR_XZEROS  DW_PR_DUx
                        " so the die tree is corrupt "
                        "(showing section, not CU, offsets). ",
                        parent_sib_val,
                        (parent_sib_val == child_overall_offset)?"at":"before",
                        child_overall_offset);
                    printf(small_buf,DW_DLV_OK,dacerr);
                }
#endif
            }

            die_stack_indent_level++;
#ifdef NOTDEF
            SET_DIE_STACK_ENTRY(die_stack_indent_level,0,dieprint_cu_goffset);
            if (die_stack_indent_level >= DIE_STACK_SIZE ) {
                printf("ERROR: compiled in DIE_STACK_SIZE limit exceeded %d dacerr: %d", DW_DLV_OK,dacerr);
            }
#endif
            print_die_and_children_internal(dbg, child, dieprint_cu_goffset, is_info, srcfiles, cnt);
#ifdef NOTDEF
            EMPTY_DIE_STACK_ENTRY(die_stack_indent_level);
#endif
            die_stack_indent_level--;
            if (die_stack_indent_level == 0) {
                local_symbols_already_began = FALSE;
            }
            dwarf_dealloc(dbg, child, DW_DLA_DIE);
            child = 0;
        } else if (cdres == DW_DLV_ERROR) {
            printf("dwarf_child cdres: %d dacerr: %d", cdres, dacerr);
        }

#ifdef NOTDEF
        /* Stop the display of all children */
        if (display_children_tree && info_flag &&
            stop_indent_level == die_stack_indent_level) {

            info_flag = FALSE;
        }
#endif

        cdres = dwarf_siblingof_b(dbg, in_die,is_info, &sibling, &dacerr);
        if (cdres == DW_DLV_OK) {
            /*  print_die_and_children(dbg, sibling, srcfiles, cnt); We
                loop around to actually print this, rather than
                recursing. Recursing is horribly wasteful of stack
                space. */
        } else if (cdres == DW_DLV_ERROR) {
            printf("dwarf_siblingof cdres: %d dacerr: %d", cdres, dacerr);
        }

#ifdef NOTDEF
        /*  If we have a sibling, verify that its offset
            is next to the last processed DIE;
            An incorrect sibling chain is a nasty bug.  */
        if (cdres == DW_DLV_OK && sibling && check_di_gaps &&
            checking_this_compiler()) {

            Dwarf_Off glb_off;
#ifdef NOTDEF
            DWARF_CHECK_COUNT(di_gaps_result,1);
#endif
            if (dwarf_validate_die_sibling(sibling,&glb_off) == DW_DLV_ERROR) {
                static char msg[128];
                Dwarf_Off sib_off;
                dwarf_dieoffset(sibling,&sib_off,&dacerr);
                sprintf(msg,
                    "GSIB = 0x%" DW_PR_XZEROS  DW_PR_DUx
                    " GOFF = 0x%" DW_PR_XZEROS DW_PR_DUx
                    " Gap = %" DW_PR_DUu " bytes",
                    sib_off,glb_off,sib_off-glb_off);
#ifdef NOTDEF
                DWARF_CHECK_ERROR2(di_gaps_result,
                    "Incorrect sibling chain",msg);
#endif
            }
        }
#endif

        /*  Here do any post-descent (ie post-dwarf_child) processing of
            the in_die. */

#ifdef NOTDEF
        EMPTY_DIE_STACK_ENTRY(die_stack_indent_level);
#endif
        if (in_die != in_die_in) {
            /*  Dealloc our in_die, but not the argument die, it belongs
                to our caller. Whether the siblingof call worked or not. */
            dwarf_dealloc(dbg, in_die, DW_DLA_DIE);
            in_die = 0;
        }
        if (cdres == DW_DLV_OK) {
            /*  Set to process the sibling, loop again. */
            in_die = sibling;
        } else {
            /*  We are done, no more siblings at this level. */
            break;
        }
    }  /* end for loop on siblings */
    return;
}

// =================================== handleOneDieSection =========================== 

static uint8_t handleOneDieSection(dwarfDbgPtr_t self, Dwarf_Debug dbg) {
  Dwarf_Unsigned cuHeaderLength = 0;
  Dwarf_Unsigned abbrev_offset = 0;
  Dwarf_Half version_stamp = 0;
  Dwarf_Half address_size = 0;
  Dwarf_Half extension_size = 0;
  Dwarf_Half length_size = 0;
  Dwarf_Sig8 signature;
  Dwarf_Unsigned typeoffset = 0;
  Dwarf_Unsigned nextCuOffset = 0;
  Dwarf_Error pod_err;
  unsigned loop_count = 0;
  int nres = DW_DLV_OK;
  int   cuCount = 0;
  char * cuShortName = NULL;
  char * cuLongName = NULL;
  const char * section_name = 0;
  int res = 0;
  Dwarf_Off dieprintCuGoffset = 0;
  int result;
  size_t cuIdx;

  /* Loop until it fails.  */
  for (;;++loop_count) {
    int sres = DW_DLV_OK;
    Dwarf_Die cuDie = 0;
    struct Dwarf_Debug_Fission_Per_CU_s fission_data;
    int fission_data_result = 0;
    Dwarf_Half cuType = 0;

    memset(&fission_data,0,sizeof(fission_data));
    nres = dwarf_next_cu_header_d(dbg, /* is_info */1, &cuHeaderLength, &version_stamp,
      &abbrev_offset, &address_size, &length_size,&extension_size, &signature, &typeoffset,
      &nextCuOffset, &cuType, &pod_err);
    if (nres != DW_DLV_OK) {
      // add info about nres somwhere!
      return DWARF_DBG_ERR_CANNOT_GET_NEXT_COMPILE_UNIT;
    }
printf("nextCuOffset: %p cuHeaderLength: %d abbrev_offset: %d address_size: %d length_size: %d\n", nextCuOffset, cuHeaderLength, abbrev_offset, address_size, length_size);
fflush(stdout);
    /*  get basic information about the current CU: producer, name */
    sres = dwarf_siblingof_b(dbg, NULL,/* is_info */1, &cuDie, &pod_err);
    if (sres != DW_DLV_OK) {
      dieprintCuGoffset = 0;
      printf("siblingof cu header sres: %d pod_err: %s", sres, pod_err);
    }
    /* Get the CU offset for easy error reporting */
    dwarf_die_offsets(cuDie, &DIEOverallOffset, &DIEOffset, &pod_err);
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
    sres = dwarf_siblingof_b(dbg, NULL, /* is_info */ 1, &cuDie, &pod_err);
    if (sres == DW_DLV_OK) {
        Dwarf_Signed cnt = 0;
        char **srcfiles = 0;
        Dwarf_Error srcerr = 0;
        int i = 0;
        int srcf = dwarf_srcfiles(cuDie, &srcfiles, &cnt, &srcerr);

        if (srcf == DW_DLV_ERROR) {
          printf("dwarf_srcfiles srcf: %d srcerr: %d", srcf, srcerr);
          dwarf_dealloc(dbg,srcerr,DW_DLA_ERROR);
          srcerr = 0;
          srcfiles = 0;
          cnt = 0;
        } /*DW_DLV_NO_ENTRY generally means there
          there is no DW_AT_stmt_list attribute.
          and we do not want to print anything
          about statements in that case */

        /* Get the CU offset for easy error reporting */
        dwarf_die_offsets(cuDie, &DIEOverallOffset, &DIEOffset, &pod_err);
        DIECuOverallOffset = DIEOverallOffset;
        DIECuOffset = DIEOffset;
        dieprintCuGoffset = DIEOverallOffset;
printf("uf: %p\n", self->dwarfDbgFileInfo->addCompileUnitFile);
        result = self->dwarfDbgFileInfo->addCompileUnitFile(self, cuShortName, DIEOverallOffset, &cuIdx);
        checkErrOK(result);
printf("cuShortName: %s DIEOverallOffset: %d DIEOffset: %d cuIdx: %d\n", cuShortName, DIEOverallOffset, DIEOffset, cuIdx);
printf("call print_die_and_children\n");
        print_die_and_children_internal(dbg, cuDie, dieprintCuGoffset, /* is_info */ 1, srcfiles, cnt);
 
        if (srcf == DW_DLV_OK) {
          int si = 0;

          // handle source (*.c) and include files (*.h)
          for (si = 0; si < cnt; ++si) {
            size_t fileIdx;
            size_t fileInfoIdx;

            result = self->dwarfDbgFileInfo->addSourceFile(self, srcfiles[si], cuIdx, &fileIdx, &fileInfoIdx);
            checkErrOK(result);
            printf("src: %d %s fileIdx: %d\n", si, srcfiles[si], fileIdx);
//            dwarf_dealloc(dbg, srcfiles[si], DW_DLA_STRING);
          }
          for (si = 0; si < cnt; ++si) {
            dwarf_dealloc(dbg, srcfiles[si], DW_DLA_STRING);
          }
          dwarf_dealloc(dbg, srcfiles, DW_DLA_LIST);
        }
    }

  }
  return DWARF_DBG_ERR_OK;
}

// =================================== dwarfDbgOpenElf =========================== 

int dwarfDbgOpenElf (dwarfDbgPtr_t self, char *fileName) {
  int dres = 0;
  Dwarf_Error onef_err = 0;

printf("dwarfDbgOpenElf\n");
fflush(stdout);
  self->elfInfo.fd = 0;
  self->elfInfo.cmd = 0;
  self->elfInfo.elf = 0;
  self->elfInfo.dbg = 0;
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
  self->elfInfo.fd = open(fileName, O_RDONLY);
  self->elfInfo.cmd = ELF_C_READ;
  self->elfInfo.elf = elf_begin(self->elfInfo.fd, self->elfInfo.cmd, (Elf *) 0);
printf("fileName: %s fd: %d arf: %p\n", fileName, self->elfInfo.fd, self->elfInfo.elf);
fflush(stdout);
  // we only handle one elf part in this version!!
  if (self->elfInfo.elf == NULL) {
    self->errorStr = "problem in elf_begin";
    return TCL_ERROR;
  }
  dres = dwarf_elf_init(self->elfInfo.elf, DW_DLC_READ, NULL, NULL, &self->elfInfo.dbg, &onef_err);
printf("dbg: %p dres: %d onef_err: %d\n", self->elfInfo.dbg, dres, onef_err);
  if (dres == DW_DLV_NO_ENTRY) {
    sprintf(self->errorBuf, "No DWARF information present in %s\n", fileName);
    self->errorStr = self->errorBuf;
    return TCL_ERROR;
  }   
  printf("dwarf_elf_init ok for %p\n", self->elfInfo.elf);
  return TCL_OK;
}

// =================================== dwarfDbgGetFiles =========================== 

int dwarfDbgGetFiles (dwarfDbgPtr_t self) {
  Dwarf_Error pod_err;
  uint8_t result;

printf("dwarfDbgGetFiles\n");
  result = handleOneDieSection(self, self->elfInfo.dbg);
  return TCL_OK;
}

// =================================== dwarfDbgCloseElf =========================== 

int dwarfDbgCloseElf (dwarfDbgPtr_t self) {
printf("dwarfDbgCloseElf\n");
printf("elf: %p\n", self->elfInfo.elf);
  elf_end(self->elfInfo.elf);
printf("fd: %d\n", self->elfInfo.fd);
  close(self->elfInfo.fd);
  return TCL_OK;
}

// =================================== dwarfDbgElfInfoInit =========================== 

int dwarfDbgElfInfoInit (dwarfDbgPtr_t self) {
printf("dwarfDbgElfInfoInit self: %p\n", self);
  _self = self;
  self->dwarfDbgElfInfo->getProducerName = getProducerName;
  self->dwarfDbgElfInfo->getCuName = getCuName;
  return DWARF_DBG_ERR_OK;
}

