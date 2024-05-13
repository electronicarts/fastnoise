///////////////////////////////////////////////////////////////////////////////
//               FastNoise - F.A.S.T. Sampling Implementation                //
//         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "technique.h"

namespace fastnoise
{
    inline void ShowToolTip(const char* tooltip)
    {
        if (!tooltip || !tooltip[0])
            return;

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
        ImGui::Text("[?]");
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("%s", tooltip);
    }

    void MakeUI(Context* context, ID3D12CommandQueue* commandQueue)
    {
        ImGui::PushID("gigi_fastnoise");

        {
            float width = ImGui::GetContentRegionAvail().x / 6.0f;
            ImGui::PushID("filterXparams");
            ImGui::PushItemWidth(width);
            ImGui::InputFloat("##X", &context->m_input.variable_filterXparams[0]);
            ImGui::SameLine();
            ImGui::InputFloat("##Y", &context->m_input.variable_filterXparams[1]);
            ImGui::SameLine();
            ImGui::InputFloat("##Z", &context->m_input.variable_filterXparams[2]);
            ImGui::SameLine();
            ImGui::InputFloat("##W", &context->m_input.variable_filterXparams[3]);
            ImGui::SameLine();
            ImGui::Text("filterXparams");
            ImGui::PopItemWidth();
            ImGui::PopID();
            ShowToolTip("");
        }
        {
            float width = ImGui::GetContentRegionAvail().x / 6.0f;
            ImGui::PushID("filterYparams");
            ImGui::PushItemWidth(width);
            ImGui::InputFloat("##X", &context->m_input.variable_filterYparams[0]);
            ImGui::SameLine();
            ImGui::InputFloat("##Y", &context->m_input.variable_filterYparams[1]);
            ImGui::SameLine();
            ImGui::InputFloat("##Z", &context->m_input.variable_filterYparams[2]);
            ImGui::SameLine();
            ImGui::InputFloat("##W", &context->m_input.variable_filterYparams[3]);
            ImGui::SameLine();
            ImGui::Text("filterYparams");
            ImGui::PopItemWidth();
            ImGui::PopID();
            ShowToolTip("");
        }
        {
            float width = ImGui::GetContentRegionAvail().x / 6.0f;
            ImGui::PushID("filterZparams");
            ImGui::PushItemWidth(width);
            ImGui::InputFloat("##X", &context->m_input.variable_filterZparams[0]);
            ImGui::SameLine();
            ImGui::InputFloat("##Y", &context->m_input.variable_filterZparams[1]);
            ImGui::SameLine();
            ImGui::InputFloat("##Z", &context->m_input.variable_filterZparams[2]);
            ImGui::SameLine();
            ImGui::InputFloat("##W", &context->m_input.variable_filterZparams[3]);
            ImGui::SameLine();
            ImGui::Text("filterZparams");
            ImGui::PopItemWidth();
            ImGui::PopID();
            ShowToolTip("");
        }
        ImGui::Checkbox("separate", &context->m_input.variable_separate);
        ShowToolTip("Whether to use "separate" mode, which makes STBN-style samples");
        ImGui::InputFloat("separateWeight", &context->m_input.variable_separateWeight);
        ShowToolTip("If "separate" is true, the weight for blending between temporal and spatial filter");
        {
            static const char* labels[] = {
                "Real",
                "Circle",
                "Vector2",
                "Vector3",
                "Vector4",
                "Sphere",
            };
            ImGui::Combo("sampleSpace", (int*)&context->m_input.variable_sampleSpace, labels, 6);
            ShowToolTip("");
        }
        {
            static const char* labels[] = {
                "Uniform1D",
                "Gauss1D",
                "Tent1D",
                "Uniform2D",
                "Uniform3D",
                "Uniform4D",
                "UniformSphere",
                "UniformHemisphere",
                "CosineHemisphere",
            };
            ImGui::Combo("sampleDistribution", (int*)&context->m_input.variable_sampleDistribution, labels, 9);
            ShowToolTip("");
        }

        ImGui::Checkbox("Profile", &context->m_profile);
        if (context->m_profile)
        {
            int numEntries = 0;
            const ProfileEntry* entries = context->ReadbackProfileData(commandQueue, numEntries);
            if (ImGui::BeginTable("profiling", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Label");
                ImGui::TableSetupColumn("CPU ms");
                ImGui::TableSetupColumn("GPU ms");
                ImGui::TableHeadersRow();
                float totalCpu = 0.0f;
                float totalGpu = 0.0f;
                for (int entryIndex = 0; entryIndex < numEntries; ++entryIndex)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(entries[entryIndex].m_label);
                    ImGui::TableNextColumn();
                    ImGui::Text("%0.3f", entries[entryIndex].m_cpu * 1000.0f);
                    ImGui::TableNextColumn();
                    ImGui::Text("%0.3f", entries[entryIndex].m_gpu * 1000.0f);
                    totalCpu += entries[entryIndex].m_cpu;
                    totalGpu += entries[entryIndex].m_gpu;
                }
                ImGui::EndTable();
            }
        }

        ImGui::PopID();
    }
};
