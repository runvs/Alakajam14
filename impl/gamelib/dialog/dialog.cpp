#include "dialog.hpp"
#include "imgui.h"

Dialog::Dialog(DialogInfo const& info)
    : m_dialogInfo { info }
{
    m_currentIndex = 0;
    m_currentLineId = m_dialogInfo.lines.front().identifier;
}

void Dialog::doUpdate(float const elapsed) { }

void Dialog::doDraw() const
{
    auto d = getCurrentLine();

    drawSingleLine(d);
    drawOptions(d);
}

void Dialog::drawOptions(DialogLine const& d) const
{
    if (m_currentIndex >= d.lines.size()) {
        openDialogue();
        if (d.lines.empty()) {
            ImGui::Text("%s", ("!!!invalid entry or empty lines: " + m_currentLineId).c_str());
            ImGui::End();
            return;
        }
        ImGui::Text("%s", d.lines.back().c_str());
        if (d.options.empty()) {
            ImGui::End();
            return;
        }

        for (auto const& opt : d.options) {
            if (ImGui::Button(opt.text.c_str())) {
                chooseOption(opt);
            }
        }
        ImGui::End();
    }
}
void Dialog::openDialogue() const
{
    ImGui::SetNextWindowSize(ImVec2 { 550, 300 });
    ImGui::Begin("Dialogue");
}
void Dialog::chooseOption(DialogOption const& opt) const
{
    resetCurrentLine();
    m_currentLineId = opt.next;
    if (m_giveSpellCallback && opt.spellToGive != "") {
        m_giveSpellCallback(opt.spellToGive);
    }
}

void Dialog::drawSingleLine(DialogLine& d) const
{
    if (m_currentIndex >= d.lines.size()) {
        return;
    }
    if (m_currentIndex == d.lines.size() - 1 && d.options.size() == 1) {
        openDialogue();
        ImGui::Text("%s", d.lines.at(m_currentIndex).c_str());
        if (ImGui::Button(d.options.at(0).text.c_str())) {
            chooseOption(d.options.at(0));
        }

        ImGui::End();
    } else {
        openDialogue();
        ImGui::Text("%s", d.lines.at(m_currentIndex).c_str());
        if (ImGui::Button("Next")) {
            nextMessageInLine();
        }

        ImGui::End();
    }
}

DialogLine Dialog::getCurrentLine() const
{
    for (auto const& l : m_dialogInfo.lines) {
        if (l.identifier == m_currentLineId) {
            return l;
        }
    }
    return DialogLine {};
}

void Dialog::nextMessageInLine() const
{
    for (auto& l : m_dialogInfo.lines) {
        if (l.identifier == m_currentLineId) {
            m_currentIndex++;
            break;
        }
    }
}

void Dialog::resetCurrentLine() const { m_currentIndex = 0; }
void Dialog::setGiveSpellCallback(std::function<void(std::string const&)> func)
{
    m_giveSpellCallback = func;
}
