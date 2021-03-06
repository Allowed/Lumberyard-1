/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
// Original file Copyright Crytek GMBH or its affiliates, used under license.

#pragma once

#include <IAudioSystem.h>

namespace Audio
{
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    class CAudioProxy
        : public IAudioProxy
    {
    public:
        CAudioProxy();
        ~CAudioProxy() override;

        // IAudioProxy
        void Initialize(const char* const sObjectName, const bool bInitAsync = true) override;
        void Release() override;
        void Reset() override;
        void ExecuteTrigger(TAudioControlID const nTriggerID, ELipSyncMethod const eLipSyncMethod, SAudioCallBackInfos const& rCallbackInfos = SAudioCallBackInfos::GetEmptyObject()) override;
        void StopAllTriggers() override;
        void StopTrigger(const TAudioControlID nTriggerID) override;
        void SetSwitchState(const TAudioControlID nSwitchID, const TAudioSwitchStateID nStateID) override;
        void SetRtpcValue(const TAudioControlID nRtpcID, const float fValue) override;
        void SetObstructionCalcType(const EAudioObjectObstructionCalcType eObstructionType) override;
        void SetPosition(const SATLWorldPosition& refPosition) override;
        void SetPosition(const Vec3& refPosition) override;
        void SetEnvironmentAmount(const TAudioEnvironmentID nEnvironmentID, const float fValue) override;
        void SetCurrentEnvironments(const EntityId nEntityToIgnore) override;
        void SetLipSyncProvider(ILipSyncProvider* const pILipSyncProvider) override;
        void ResetRtpcValues() override;
        TAudioObjectID GetAudioObjectID() const override
        {
            return m_nAudioObjectID;
        }
        // ~IAudioProxy

        static void OnAudioEvent(const SAudioRequestInfo* const pAudioRequestInfo);

        void ClearEnvironments();
        void ExecuteQueuedCommands();

    private:
        static const size_t sMaxAreas = 10;

        TAudioObjectID m_nAudioObjectID;
        SATLWorldPosition m_oPosition;

        TATLEnumFlagsType m_nFlags;

        ILipSyncProviderPtr m_oLipSyncProvider;
        ELipSyncMethod m_eCurrentLipSyncMethod;
        TAudioControlID m_nCurrentLipSyncID;

        ///////////////////////////////////////////////////////////////////////////////////////////////
        enum EQueuedAudioCommandType : TATLEnumFlagsType
        {
            eQACT_NONE                      = 0,
            eQACT_EXECUTE_TRIGGER,
            eQACT_STOP_TRIGGER,
            eQACT_SET_SWITCH_STATE,
            eQACT_SET_RTPC_VALUE,
            eQACT_SET_POSITION,
            eQACT_SET_ENVIRONMENT_AMOUNT,
            eQACT_SET_CURRENT_ENVIRONMENTS,
            eQACT_CLEAR_ENVIRONMENTS,
            eQACT_CLEAR_RTPCS,
            eQACT_RESET,
            eQACT_RELEASE,
            eQACT_INITIALIZE,
            eQACT_STOP_ALL_TRIGGERS,
        };

        ///////////////////////////////////////////////////////////////////////////////////////////////
        struct SQueuedAudioCommand
        {
            SQueuedAudioCommand(const EQueuedAudioCommandType ePassedType)
                : eType(ePassedType)
                , nTriggerID(INVALID_AUDIO_CONTROL_ID)
                , nSwitchID(INVALID_AUDIO_CONTROL_ID)
                , nStateID(INVALID_AUDIO_SWITCH_STATE_ID)
                , nRtpcID(INVALID_AUDIO_CONTROL_ID)
                , fValue(0.0f)
                , nEnvironmentID(INVALID_AUDIO_ENVIRONMENT_ID)
                , eLipSyncMethod(eLSM_None)
                , nEntityID(((EntityId)(0)))
                , pOwnerOverride(nullptr)
                , pUserData(nullptr)
                , pUserDataOwner(nullptr)
                , nRequestFlags(eARF_NONE)
            {}

            SQueuedAudioCommand& operator=(const SQueuedAudioCommand& refOther)
            {
                const_cast<EQueuedAudioCommandType&>(eType) = refOther.eType;
                nTriggerID = refOther.nTriggerID;
                nSwitchID = refOther.nSwitchID;
                nStateID = refOther.nStateID;
                nRtpcID = refOther.nRtpcID;
                fValue = refOther.fValue;
                nEnvironmentID = refOther.nEnvironmentID;
                eLipSyncMethod = refOther.eLipSyncMethod;
                nEntityID = refOther.nEntityID;
                pOwnerOverride = refOther.pOwnerOverride;
                pUserData = refOther.pUserData;
                pUserDataOwner = refOther.pUserDataOwner;
                nRequestFlags = refOther.nRequestFlags;
                sValue = refOther.sValue;
                oPosition = refOther.oPosition;
                return *this;
            }

            const EQueuedAudioCommandType eType;
            TAudioControlID nTriggerID;
            TAudioControlID nSwitchID;
            TAudioSwitchStateID nStateID;
            TAudioControlID nRtpcID;
            float fValue;
            TAudioEnvironmentID nEnvironmentID;
            ELipSyncMethod eLipSyncMethod;
            EntityId nEntityID;
            void* pOwnerOverride;
            void* pUserData;
            void* pUserDataOwner;
            TATLEnumFlagsType nRequestFlags;
            string sValue;
            SATLWorldPosition oPosition;
        };

        ///////////////////////////////////////////////////////////////////////////////////////////////
        enum EAudioProxyFlags : TATLEnumFlagsType
        {
            eAPF_NONE           = 0,
            eAPF_WAITING_FOR_ID = BIT(0),
        };

        using TQueuedAudioCommands = AZStd::deque<SQueuedAudioCommand, Audio::AudioSystemStdAllocator>;
        TQueuedAudioCommands m_aQueuedAudioCommands;

        void TryAddQueuedCommand(const SQueuedAudioCommand& refCommand);

        //////////////////////////////////////////////////////////////////////////
        struct SFindSetSwitchState
        {
            SFindSetSwitchState(const TAudioControlID passedAudioSwitchID, const TAudioSwitchStateID passedAudioSwitchStateID)
                : audioSwitchID(passedAudioSwitchID)
                , audioSwitchStateID(passedAudioSwitchStateID)
            {}

            bool operator()(SQueuedAudioCommand& refCommand)
            {
                bool bFound = false;

                if (refCommand.eType == eQACT_SET_SWITCH_STATE && refCommand.nSwitchID == audioSwitchID)
                {
                    // Command for this switch exists, just update the state.
                    refCommand.nStateID = audioSwitchStateID;
                    bFound = true;
                }

                return bFound;
            }

        private:

            const TAudioControlID audioSwitchID;
            const TAudioSwitchStateID audioSwitchStateID;
        };

        //////////////////////////////////////////////////////////////////////////
        struct SFindSetRtpcValue
        {
            SFindSetRtpcValue(const TAudioControlID passedAudioRtpcID, const float passedAudioRtpcValue)
                : audioRtpcID(passedAudioRtpcID)
                , audioRtpcValue(passedAudioRtpcValue)
            {}

            bool operator()(SQueuedAudioCommand& refCommand)
            {
                bool bFound = false;

                if (refCommand.eType == eQACT_SET_RTPC_VALUE && refCommand.nRtpcID == audioRtpcID)
                {
                    // Command for this RTPC exists, just update the value.
                    refCommand.fValue = audioRtpcValue;
                    bFound = true;
                }

                return bFound;
            }

        private:

            const TAudioControlID audioRtpcID;
            const float audioRtpcValue;
        };

        //////////////////////////////////////////////////////////////////////////
        struct SFindSetPosition
        {
            SFindSetPosition(const SATLWorldPosition& refPassedPosition)
                : refPosition(refPassedPosition)
            {}

            bool operator()(SQueuedAudioCommand& refCommand)
            {
                bool bFound = false;

                if (refCommand.eType == eQACT_SET_POSITION)
                {
                    // Command exists already, just update the position.
                    refCommand.oPosition = refPosition;
                    bFound = true;
                }

                return bFound;
            }

        private:

            const SATLWorldPosition& refPosition;
        };

        //////////////////////////////////////////////////////////////////////////
        struct SFindSetEnvironmentAmount
        {
            SFindSetEnvironmentAmount(const TAudioEnvironmentID passedAudioEnvironmentID, const float passedAudioEnvironmentValue)
                : audioEnvironmentID(passedAudioEnvironmentID)
                , audioEnvironmentValue(passedAudioEnvironmentValue)
            {}

            bool operator()(SQueuedAudioCommand& refCommand)
            {
                bool bFound = false;

                if (refCommand.eType == eQACT_SET_ENVIRONMENT_AMOUNT && refCommand.nEnvironmentID == audioEnvironmentID)
                {
                    // Command for this environment exists, just update the value.
                    refCommand.fValue = audioEnvironmentValue;
                    bFound = true;
                }

                return bFound;
            }

        private:

            const TAudioEnvironmentID audioEnvironmentID;
            const float audioEnvironmentValue;
        };

        //////////////////////////////////////////////////////////////////////////
        struct SFindCommand
        {
            SFindCommand(const EQueuedAudioCommandType passedAudioQueuedCommandType)
                : audioQueuedCommandType(passedAudioQueuedCommandType)
            {}

            bool operator()(const SQueuedAudioCommand& refCommand)
            {
                return refCommand.eType == audioQueuedCommandType;
            }

        private:

            const EQueuedAudioCommandType audioQueuedCommandType;
        };
    };
} // namespace Audio
