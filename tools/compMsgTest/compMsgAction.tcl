# ===========================================================================
# * Copyright (c) 2016, Arnulf P. Wiedemann (arnulf@wiedemann-pri.de)
# * All rights reserved.
# *
# * License: BSD/MIT
# *
# * Redistribution and use in source and binary forms, with or without
# * modification, are permitted provided that the following conditions
# * are met:
# *
# * 1. Redistributions of source code must retain the above copyright
# * notice, this list of conditions and the following disclaimer.
# * 2. Redistributions in binary form must reproduce the above copyright
# * notice, this list of conditions and the following disclaimer in the
# * documentation and/or other materials provided with the distribution.
# * 3. Neither the name of the copyright holder nor the names of its
# * contributors may be used to endorse or promote products derived
# * from this software without specific prior written permission.
# *
# * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# * POSSIBILITY OF SUCH DAMAGE.
# *
# ==========================================================================

# actionName2Action dict
#   actionName
#   action
#   u16CmdKey
#   u8CmdKey
#   mode

# compMsgActionEntries dict
#   actionEntries
#   numActionEntries
#   maxActionEntries

# compMsgActions dict
#   actions
#   numActions
#   maxActions

set ::COMP_MSG_ACTIONS_FILE_NAME "CompMsgActions.txt"

namespace eval compMsg {
  namespace ensemble create

    namespace export compMsgAction

  namespace eval compMsgAction {
    namespace ensemble create
      
    namespace export getActionCallbackName compMsgActionInit setActionEntry

      variable compMsgActionEntries
      variable compMsgActions

    # ================================= runClientMode ====================================
    
    proc runClientMode {compMsgDispatcherVar mode} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      compMsgDispatcher websocketRunClientMode{cmdisp, mode};
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= runAPMode ====================================
    
   proc runAPMode {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      compMsgDispatcher websocketRunAPMode{cmdisp};
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= runLightSleepWakeupMode ====================================
    
    proc runLightSleepWakeupMode {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= runLightSleepNoWakeupMode ====================================
    
    proc runLightSleepNoWakeupMode {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= runWpsMode ====================================
    
    proc runWpsMode {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= runModulTestMode ====================================
    
    proc runModulTestMode {compMsgDispatcherBVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher

      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= runDeletePasswdCMode ====================================
    
    proc runDeletePasswdCMode {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
puts stderr "runDeletePasswdC"
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getAPList ====================================
    
    proc getAPList {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      set result [::compMsg compMsgDispatcher getBssScanInfo{cmdisp};
      checkErrOK $result
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getTableValue ====================================
    
    proc getTableValue {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      set result [::compMsg compMsgDispatcher getModuleTableFieldValue{cmdisp, MODULE_INFO_AP_LIST_CALL_BACK};
      checkErrOK $result
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getWifiKeyValueInfos ====================================
    
    proc getWifiKeyValueInfos {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      set result [::compMsg compMsgDispatcher getWifiKeyValueInfo{cmdisp};
      checkErrOK $result
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getWifiKeyValues ====================================
    
    proc getWifiKeyValues {compMsgDispatcherVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      set result [::compMsg compMsgDispatcher getWifiKeyValue{cmdisp};
      checkErrOK $result
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= getActionMode ====================================
    
    proc getActionMode {compMsgDispatcherVar actionName actionMode} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      setidx 0
      actionEntry = &actionName2Actions[idx]
      while {actionEntry->actionName != NULL} { 
        if {c_strcmp{actionEntry->actionName, actionName} == 0} {
          *actionMode = actionEntry->mode
    ets_printf{"actionMode: %d\n", *actionMode}
          return $::COMP_DISP_ERR_OK
        }
        idx++;
        actionEntry = &actionName2Actions[idx]
      }
      checkErrOK $::COMP_DISP_ERR_ACTION_NAME_NOT_FOUND
    }
    
    # ================================= getActionCallback ====================================
    
    proc getActionCallback {compMsgDispatcherVar actionName callback} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      idx = 0;
      actionEntry = &actionName2Actions[idx];
      while {actionEntry->actionName != NULL} { 
        if {c_strcmp{actionEntry->actionName, actionName} == 0} {
          *callback = actionEntry->action;
          return $::COMP_DISP_ERR_OK;
        }
        idx++;
        actionEntry = &actionName2Actions[idx];
      }
      checkErrOK $::COMP_DISP_ERR_ACTION_NAME_NOT_FOUND;
    }
    
    # ================================= getActionCallbackName ====================================
    
    proc getActionCallbackName {compMsgDispatcherVar callback actionNameVar} {
      upvar $compMsgDispatcherVar compMsgDispatcher
      upvar $actionNameVar actionName
    
      if {[string range $callback 0 0] eq "@"} {
        set actionName [string range $callback 1 end]
      } else {
        set actionName $callback
      }
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= setActionEntry ====================================
    
    proc setActionEntry {compMsgDispatcherVar actionName mode cmdKey} {
      variable compMsgActionEntries
      upvar $compMsgDispatcherVar compMsgDispatcher
    
#puts stderr "setActionEntry $actionName $mode $cmdKey!"
if {0} {
      if {[dict get $compMsgActionEntries numActionEntries] >= [dict get $compMsgActionEntries maxActionEntries]} {
      }
      set idx 0
      set actionEntry  &actionName2Actions[idx];
      while {[dict get $actionEntry actionName] != NULL} { 
        if {c_strcmp{actionEntry->actionName, actionName} == 0} {
          compMsgActionEntries.actionEntries[compMsgActionEntries.numActionEntries] = actionEntry
          if {actionEntry->mode != 0} {
            checkErrOK $::COMP_DISP_ERR_DUPLICATE_ENTRY
          }
          actionEntry->mode = mode
          actionEntry->u8CmdKey = u8CmdKey
          actionEntry->u16CmdKey = u16CmdKey
          compMsgActionEntries.numActionEntries++
          return $::COMP_DISP_ERR_OK
        }
        incr idx
        actionEntry = &actionName2Actions[idx]
      }
      return $::COMP_DISP_ERR_ACTION_NAME_NOT_FOUND
}
      return $::COMP_DISP_ERR_OK
    }
    
    # ================================= runAction ====================================
    
    proc runAction {compMsgDispatcherVar answerType} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      received = &compMsgDispatcher received;
      dataView = compMsgDispatcher compMsgDataView->dataView;
      if {received->u16CmdKey == 0x4244} { # "BD"
        # FIXME need to get the real offset here instead of 7!!
        set result [::compMsg dataView->getUint8{dataView, 7, &actionMode};
        checkErrOK $result
        idx = 0;
        actionEntry = &actionName2Actions[idx];
        while {actionEntry->actionName != NULL} { 
    #ets_printf{"runActionBu8!%s!%c!%c!%c!", actionEntry->actionName, {received->u16CmdKey>>8}&0xFF, received->u16CmdKey&0xFF, actionMode};
          if {{actionEntry->u16CmdKey == received->u16CmdKey} && {actionMode == actionEntry->mode}} {
    ets_printf{"runAction!%s!%d!", actionEntry->actionName, actionEntry->mode};
            set result [::compMsg actionEntry->action{compMsgDispatcher};
            checkErrOK{result};
            return $::COMP_DISP_ERR_OK
          }
          idx++;
          actionEntry = &actionName2Actions[idx];
        }
        checkErrOK $::COMP_DISP_ERR_ACTION_NAME_NOT_FOUND
      } else {
    #ets_printf{"runAction u16!%c%c!%c!\n", {received->u16CmdKey>>8}&0xFF, received->u16CmdKey&0xFF, *answerType};
        dataView = compMsgDispatcher compMsgDataView->dataView;
        switch {compMsgDispatcher actionMode} {
        case 8:
        case MODULE_INFO_AP_LIST_CALL_BACK:
          idx = 0;
          actionEntry = &actionName2Actions[idx];
          while {actionEntry->actionName != NULL} { 
    #ets_printf{"an2: %s am: %d %d\n", actionEntry->actionName, actionMode, actionEntry->mode};
            if {compMsgDispatcher actionMode == actionEntry->mode} {
    #ets_printf{"runAction2 G!%d!%c!\n", compMsgDispatcher actionMode, *answerType};
    ets_printf{"runAction!%s!%d!", actionEntry->actionName, actionEntry->mode};
              set result [::compMsg actionEntry->action{compMsgDispatcher};
              checkErrOK{result};
              return $::COMP_DISP_ERR_OK
            }
            idx++;
            actionEntry = &actionName2Actions[idx];
          }
          checkErrOK $::COMP_DISP_ERR_ACTION_NAME_NOT_FOUND
          break;
        }
      }
      checkErrOK $::COMP_DISP_ERR_ACTION_NAME_NOT_FOUND
    }
    
    # ================================= fillMsgValue ====================================
    
    proc fillMsgValue {compMsgDispatcherVar callbackName answerType fieldTypeId} {
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      # skip the '@' char!
      callbackName++;
      idx = 0;
      actionEntry = &actionName2Actions[idx];
      while {actionEntry->actionName != NULL} { 
        if {c_strcmp{actionEntry->actionName, callbackName} == 0} {
          set result [::compMsg actionEntry->action{compMsgDispatcher};
          checkErrOK{result};
          return $::COMP_DISP_ERR_OK
        }
        idx++;
        actionEntry = &actionName2Actions[idx];
      }
      checkErrOK $::COMP_DISP_ERR_ACTION_NAME_NOT_FOUND
    }
    
    # ================================= compMsgActionInit ====================================
    
    proc compMsgActionInit {compMsgDispatcherVar} {
      variable compMsgActionEntries
      variable compMsgActions
      upvar $compMsgDispatcherVar compMsgDispatcher
    
      set compMsgActionEntries [dict create]
      dict set compMsgActionEntries numActionEntries 0
      dict set compMsgActionEntries maxActionEntries 10
      dict set compMsgActionEntries actionEntries [list]
    
      set compMsgActions [dict create]
      dict set compMsgActions numActions 0
      dict set compMsgActions maxActions 10
      dict set compMsgActions actions [list]
    
      set result [::compMsg compMsgMsgDesc readActions compMsgDispatcher $::COMP_MSG_ACTIONS_FILE_NAME]
      checkErrOK $result
      return $::COMP_DISP_ERR_OK
    }

  } ; # namespace compMsgAction
} ; # namespace compMsg
