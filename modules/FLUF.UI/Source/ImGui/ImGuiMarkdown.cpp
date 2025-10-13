#include <PCH.hpp>

#include "ImGui/ImGuiMarkdown.hpp"

#include "ImGui/ImGuiInterface.hpp"

/*
 * imgui_md: Markdown for Dear ImGui using MD4C
 * (http://https://github.com/mekhontsev/imgui_md)
 *
 * Copyright (c) 2021 Dmitry Mekhontsev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

ImguiMarkdown::ImguiMarkdown(ImGuiInterface* imguiInterface) : imguiInterface(imguiInterface)
{
    parser.abi_version = 0;

    parser.flags = MD_FLAG_TABLES | MD_FLAG_UNDERLINE | MD_FLAG_STRIKETHROUGH;

    parser.enter_block = [](MD_BLOCKTYPE t, void* d, void* u) { return ((ImguiMarkdown*)u)->Block(t, d, true); };

    parser.leave_block = [](MD_BLOCKTYPE t, void* d, void* u) { return ((ImguiMarkdown*)u)->Block(t, d, false); };

    parser.enter_span = [](MD_SPANTYPE t, void* d, void* u) { return ((ImguiMarkdown*)u)->Span(t, d, true); };

    parser.leave_span = [](MD_SPANTYPE t, void* d, void* u) { return ((ImguiMarkdown*)u)->Span(t, d, false); };

    parser.text = [](MD_TEXTTYPE t, const MD_CHAR* text, MD_SIZE size, void* u) { return ((ImguiMarkdown*)u)->Text(t, text, text + size); };

    parser.debug_log = nullptr;

    parser.syntax = nullptr;

    ////////////////////////////////////////////////////////////////////////////

    tableLastPos = ImVec2(0, 0);
}

void ImguiMarkdown::BLOCK_UL(const MD_BLOCK_UL_DETAIL* d, bool e)
{
    if (e)
    {
        listStack.push_back(ListInfo{ 0, d->mark, false });
    }
    else
    {
        listStack.pop_back();
        if (listStack.empty())
        {
            ImGui::NewLine();
        }
    }
}

void ImguiMarkdown::BLOCK_OL(const MD_BLOCK_OL_DETAIL* d, bool e)
{
    if (e)
    {
        listStack.push_back(ListInfo{ d->start, d->mark_delimiter, true });
    }
    else
    {
        listStack.pop_back();
        if (listStack.empty())
        {
            ImGui::NewLine();
        }
    }
}

void ImguiMarkdown::BLOCK_LI(const MD_BLOCK_LI_DETAIL*, bool e)
{
    if (e)
    {
        ImGui::NewLine();

        ListInfo& nfo = listStack.back();
        if (nfo.isOl)
        {
            ImGui::Text("%d%c", nfo.currentOl++, nfo.delim);
            ImGui::SameLine();
        }
        else
        {
            if (nfo.delim == '*')
            {
                float cx = ImGui::GetCursorPosX();
                cx -= ImGui::GetStyle().FramePadding.x * 2;
                ImGui::SetCursorPosX(cx);
                ImGui::Bullet();
            }
            else
            {
                ImGui::Text("%c", nfo.delim);
                ImGui::SameLine();
            }
        }

        ImGui::Indent();
    }
    else
    {
        ImGui::Unindent();
    }
}

void ImguiMarkdown::BLOCK_HR(bool e)
{
    if (!e)
    {
        ImGui::NewLine();
        ImGui::Separator();
    }
}

void ImguiMarkdown::BLOCK_H(const MD_BLOCK_H_DETAIL* d, bool e)
{
    if (e)
    {
        headingLevel = d->level;
        ImGui::NewLine();
    }
    else
    {
        headingLevel = 0;
    }

    SetFont(e);

    if (!e)
    {
        if (d->level <= 2)
        {
            ImGui::NewLine();
            ImGui::Separator();
        }
    }
}

void ImguiMarkdown::BLOCK_DOC(bool) {}
void ImguiMarkdown::BLOCK_QUOTE(bool) {}
void ImguiMarkdown::BLOCK_CODE(const MD_BLOCK_CODE_DETAIL*, bool e) { isCode = e; }

void ImguiMarkdown::BLOCK_HTML(bool) {}

void ImguiMarkdown::BLOCK_P(bool)
{
    if (!listStack.empty())
    {
        return;
    }
    ImGui::NewLine();
}

void ImguiMarkdown::BLOCK_TABLE(const MD_BLOCK_TABLE_DETAIL*, bool e)
{
    if (e)
    {
        tableRowPos.clear();
        tableColPos.clear();

        tableLastPos = ImGui::GetCursorPos();
    }
    else
    {

        ImGui::NewLine();

        if (tableBorder)
        {

            tableLastPos.y = ImGui::GetCursorPos().y;

            tableColPos.push_back(tableLastPos.x);
            tableRowPos.push_back(tableLastPos.y);

            const ImVec2 wp = ImGui::GetWindowPos();
            const ImVec2 sp = ImGui::GetStyle().ItemSpacing;
            const float wx = wp.x + sp.x / 2;
            const float wy = wp.y - sp.y / 2 - ImGui::GetScrollY();

            for (int i = 0; i < tableColPos.size(); ++i)
            {
                tableColPos[i] += wx;
            }

            for (int i = 0; i < tableRowPos.size(); ++i)
            {
                tableRowPos[i] += wy;
            }

            ////////////////////////////////////////////////////////////////////

            const ImColor c = ImGui::GetStyle().Colors[ImGuiCol_TextDisabled];

            ImDrawList* dl = ImGui::GetWindowDrawList();

            const float xmin = tableColPos.front();
            const float xmax = tableColPos.back();
            for (int i = 0; i < tableRowPos.size(); ++i)
            {
                const float p = tableRowPos[i];
                dl->AddLine(ImVec2(xmin, p), ImVec2(xmax, p), c, i == 1 && tableHeaderHighlight ? 2.0f : 1.0f);
            }

            const float ymin = tableRowPos.front();
            const float ymax = tableRowPos.back();
            for (int i = 0; i < tableColPos.size(); ++i)
            {
                const float p = tableColPos[i];
                dl->AddLine(ImVec2(p, ymin), ImVec2(p, ymax), c, 1.0f);
            }
        }
    }
}

void ImguiMarkdown::BLOCK_THEAD(bool e)
{
    isTableHeader = e;
    if (tableHeaderHighlight)
    {
        SetFont(e);
    }
}

void ImguiMarkdown::BLOCK_TBODY(bool e) { isTableBody = e; }

void ImguiMarkdown::BLOCK_TR(bool e)
{
    ImGui::SetCursorPosY(tableLastPos.y);

    if (e)
    {
        tableNextColumn = 0;
        ImGui::NewLine();
        tableRowPos.push_back(ImGui::GetCursorPosY());
    }
}

void ImguiMarkdown::BLOCK_TH(const MD_BLOCK_TD_DETAIL* d, bool e) { BLOCK_TD(d, e); }

void ImguiMarkdown::BLOCK_TD(const MD_BLOCK_TD_DETAIL*, bool e)
{
    if (e)
    {

        if (tableNextColumn < tableColPos.size())
        {
            ImGui::SetCursorPosX(tableColPos[tableNextColumn]);
        }
        else
        {
            tableColPos.push_back(tableLastPos.x);
        }

        ++tableNextColumn;

        ImGui::Indent(tableColPos[tableNextColumn - 1]);
        ImGui::SetCursorPos(ImVec2(tableColPos[tableNextColumn - 1], tableRowPos.back()));
    }
    else
    {
        const ImVec2 p = ImGui::GetCursorPos();
        ImGui::Unindent(tableColPos[tableNextColumn - 1]);
        ImGui::SetCursorPosX(p.x);
        if (p.y > tableLastPos.y)
        {
            tableLastPos.y = p.y;
        }
    }
    ImGui::TextUnformatted("");

    if (!tableBorder && e && tableNextColumn == 1)
    {
        ImGui::SameLine(0.0f, 0.0f);
    }
    else
    {
        ImGui::SameLine();
    }
}

////////////////////////////////////////////////////////////////////////////////
void ImguiMarkdown::SetHRef(bool e, const MD_ATTRIBUTE& src)
{
    if (e)
    {
        href.assign(src.text, src.size);
    }
    else
    {
        href.clear();
    }
}

void ImguiMarkdown::SetFont(bool e)
{
    if (e)
    {
        ImGui::PushFont(GetFont());
    }
    else
    {
        ImGui::PopFont();
    }
}

void ImguiMarkdown::SetColor(bool e)
{
    if (e)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, GetColor());
    }
    else
    {
        ImGui::PopStyleColor();
    }
}

void ImguiMarkdown::Line(ImColor c, bool under)
{
    ImVec2 mi = ImGui::GetItemRectMin();
    ImVec2 ma = ImGui::GetItemRectMax();

    if (!under)
    {
        ma.y -= ImGui::GetFontSize() / 2;
    }

    mi.y = ma.y;

    ImGui::GetWindowDrawList()->AddLine(mi, ma, c, 1.0f);
}

void ImguiMarkdown::SPAN_A(const MD_SPAN_A_DETAIL* d, bool e)
{
    SetHRef(e, d->href);
    SetColor(e);
}

void ImguiMarkdown::SPAN_EM(bool e)
{
    isEm = e;
    SetFont(e);
}

void ImguiMarkdown::SPAN_STRONG(bool e)
{
    isStrong = e;
    SetFont(e);
}

void ImguiMarkdown::SPAN_IMG(const MD_SPAN_IMG_DETAIL* d, bool e)
{
    isImage = e;

    SetHRef(e, d->src);

    if (e)
    {

        image_info nfo;
        if (GetImage(nfo))
        {

            const float scale = ImGui::GetIO().FontGlobalScale;
            nfo.size.x *= scale;
            nfo.size.y *= scale;

            const ImVec2 csz = ImGui::GetContentRegionAvail();
            if (nfo.size.x > csz.x)
            {
                const float r = nfo.size.y / nfo.size.x;
                nfo.size.x = csz.x;
                nfo.size.y = csz.x * r;
            }

            ImGui::Image(nfo.textureId, nfo.size, nfo.uv0, nfo.uv1, nfo.colTint, nfo.colBorder);

            if (ImGui::IsItemHovered())
            {

                //if (d->title.size) {
                //	ImGui::SetTooltip("%.*s", (int)d->title.size, d->title.text);
                //}

                if (ImGui::IsMouseReleased(0))
                {
                    OpenUrl();
                }
            }
        }
    }
}

void ImguiMarkdown::SPAN_CODE(bool) {}

void ImguiMarkdown::SPAN_LATEXMATH(bool) {}

void ImguiMarkdown::SPAN_LATEXMATH_DISPLAY(bool) {}

void ImguiMarkdown::SPAN_WIKILINK(const MD_SPAN_WIKILINK_DETAIL*, bool) {}

void ImguiMarkdown::SPAN_U(bool e) { isUnderline = e; }

void ImguiMarkdown::SPAN_DEL(bool e) { isStrikethrough = e; }

void ImguiMarkdown::RenderText(const char* str, const char* str_end)
{
    const float scale = ImGui::GetIO().FontGlobalScale;
    const ImGuiStyle& s = ImGui::GetStyle();
    bool is_lf = false;

    while (!isImage && str < str_end)
    {

        const char* te = str_end;

        if (!isTableHeader)
        {

            float wl = ImGui::GetContentRegionAvail().x;

            if (isTableBody)
            {
                wl = (tableNextColumn < tableColPos.size() ? tableColPos[tableNextColumn] : tableLastPos.x);
                wl -= ImGui::GetCursorPosX();
            }

            te = ImGui::GetFont()->CalcWordWrapPositionA(scale, str, str_end, wl);

            if (te == str)
            {
                ++te;
            }
        }

        ImGui::TextUnformatted(str, te);

        if (te > str && *(te - 1) == '\n')
        {
            is_lf = true;
        }

        if (!href.empty())
        {

            ImVec4 c;
            if (ImGui::IsItemHovered())
            {

                ImGui::SetTooltip("%s", href.c_str());

                c = s.Colors[ImGuiCol_ButtonHovered];
                if (ImGui::IsMouseReleased(0))
                {
                    OpenUrl();
                }
            }
            else
            {
                c = s.Colors[ImGuiCol_Button];
            }
            Line(c, true);
        }
        if (isUnderline)
        {
            Line(s.Colors[ImGuiCol_Text], true);
        }
        if (isStrikethrough)
        {
            Line(s.Colors[ImGuiCol_Text], false);
        }

        str = te;

        while (str < str_end && *str == ' ')
        {
            ++str;
        }
    }

    if (!is_lf)
    {
        ImGui::SameLine(0.0f, 0.0f);
    }
}

bool ImguiMarkdown::RenderEntity(const char* str, const char* str_end)
{
    const size_t sz = str_end - str;
    if (strncmp(str, "&nbsp;", sz) == 0)
    {
        ImGui::TextUnformatted("");
        ImGui::SameLine();
        return true;
    }
    return false;
}

static bool skip_spaces(const std::string& d, size_t& p)
{
    for (; p < d.length(); ++p)
    {
        if (d[p] != ' ' && d[p] != '\t')
        {
            break;
        }
    }
    return p < d.length();
}

static std::string get_div_class(const char* str, const char* str_end)
{
    if (str_end <= str)
    {
        return "";
    }

    std::string d(str, str_end - str);
    if (d.back() == '>')
    {
        d.pop_back();
    }

    const char attr[] = "class";
    size_t p = d.find(attr);
    if (p == std::string::npos)
    {
        return "";
    }
    p += sizeof(attr) - 1;

    if (!skip_spaces(d, p))
    {
        return "";
    }

    if (d[p] != '=')
    {
        return "";
    }
    ++p;

    if (!skip_spaces(d, p))
    {
        return "";
    }

    bool has_q = false;

    if (d[p] == '"' || d[p] == '\'')
    {
        has_q = true;
        ++p;
    }
    if (p == d.length())
    {
        return "";
    }

    if (!has_q)
    {
        if (!skip_spaces(d, p))
        {
            return "";
        }
    }

    size_t pe;
    for (pe = p; pe < d.length(); ++pe)
    {

        const char c = d[pe];

        if (has_q)
        {
            if (c == '"' || c == '\'')
            {
                break;
            }
        }
        else
        {
            if (c == ' ' || c == '\t')
            {
                break;
            }
        }
    }

    return d.substr(p, pe - p);
}

bool ImguiMarkdown::CheckHtml(const char* str, const char* str_end)
{
    const size_t sz = str_end - str;

    if (strncmp(str, "<br>", sz) == 0)
    {
        ImGui::NewLine();
        return true;
    }

    if (strncmp(str, "<hr>", sz) == 0)
    {
        ImGui::Separator();
        return true;
    }

    if (strncmp(str, "<u>", sz) == 0)
    {
        isUnderline = true;
        return true;
    }

    if (strncmp(str, "</u>", sz) == 0)
    {
        isUnderline = false;
        return true;
    }

    const size_t div_sz = 4;
    if (strncmp(str, "<div", sz > div_sz ? div_sz : sz) == 0)
    {
        divStack.emplace_back(get_div_class(str + div_sz, str_end));
        HtmlDiv(divStack.back(), true);
        return true;
    }

    if (strncmp(str, "</div>", sz) == 0)
    {
        if (divStack.empty())
        {
            return false;
        }
        HtmlDiv(divStack.back(), false);
        divStack.pop_back();
        return true;
    }

    return false;
}

void ImguiMarkdown::HtmlDiv(const std::string& dclass, bool e)
{
    //Example:
#if 0
	if (dclass == "red") {
		if (e) {
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
		} else {
			ImGui::PopStyleColor();
		}
	}
#endif
}

int ImguiMarkdown::Text(MD_TEXTTYPE type, const char* str, const char* str_end)
{
    switch (type)
    {
        case MD_TEXT_NORMAL: RenderText(str, str_end); break;
        case MD_TEXT_CODE: RenderText(str, str_end); break;
        case MD_TEXT_NULLCHAR: break;
        case MD_TEXT_BR: ImGui::NewLine(); break;
        case MD_TEXT_SOFTBR: SoftBreak(); break;
        case MD_TEXT_ENTITY:
            if (!RenderEntity(str, str_end))
            {
                RenderText(str, str_end);
            };
            break;
        case MD_TEXT_HTML:
            if (!CheckHtml(str, str_end))
            {
                RenderText(str, str_end);
            }
            break;
        case MD_TEXT_LATEXMATH: RenderText(str, str_end); break;
        default: break;
    }

    if (isTableHeader)
    {
        const float x = ImGui::GetCursorPosX();
        if (x > tableLastPos.x)
        {
            tableLastPos.x = x;
        }
    }

    return 0;
}

int ImguiMarkdown::Block(MD_BLOCKTYPE type, void* d, bool e)
{
    switch (type)
    {
        case MD_BLOCK_DOC: BLOCK_DOC(e); break;
        case MD_BLOCK_QUOTE: BLOCK_QUOTE(e); break;
        case MD_BLOCK_UL: BLOCK_UL((MD_BLOCK_UL_DETAIL*)d, e); break;
        case MD_BLOCK_OL: BLOCK_OL((MD_BLOCK_OL_DETAIL*)d, e); break;
        case MD_BLOCK_LI: BLOCK_LI((MD_BLOCK_LI_DETAIL*)d, e); break;
        case MD_BLOCK_HR: BLOCK_HR(e); break;
        case MD_BLOCK_H: BLOCK_H((MD_BLOCK_H_DETAIL*)d, e); break;
        case MD_BLOCK_CODE: BLOCK_CODE((MD_BLOCK_CODE_DETAIL*)d, e); break;
        case MD_BLOCK_HTML: BLOCK_HTML(e); break;
        case MD_BLOCK_P: BLOCK_P(e); break;
        case MD_BLOCK_TABLE: BLOCK_TABLE((MD_BLOCK_TABLE_DETAIL*)d, e); break;
        case MD_BLOCK_THEAD: BLOCK_THEAD(e); break;
        case MD_BLOCK_TBODY: BLOCK_TBODY(e); break;
        case MD_BLOCK_TR: BLOCK_TR(e); break;
        case MD_BLOCK_TH: BLOCK_TH((MD_BLOCK_TD_DETAIL*)d, e); break;
        case MD_BLOCK_TD: BLOCK_TD((MD_BLOCK_TD_DETAIL*)d, e); break;
        default: assert(false); break;
    }

    return 0;
}

int ImguiMarkdown::Span(MD_SPANTYPE type, void* d, bool e)
{
    switch (type)
    {
        case MD_SPAN_EM: SPAN_EM(e); break;
        case MD_SPAN_STRONG: SPAN_STRONG(e); break;
        case MD_SPAN_A: SPAN_A((MD_SPAN_A_DETAIL*)d, e); break;
        case MD_SPAN_IMG: SPAN_IMG((MD_SPAN_IMG_DETAIL*)d, e); break;
        case MD_SPAN_CODE: SPAN_CODE(e); break;
        case MD_SPAN_DEL: SPAN_DEL(e); break;
        case MD_SPAN_LATEXMATH: SPAN_LATEXMATH(e); break;
        case MD_SPAN_LATEXMATH_DISPLAY: SPAN_LATEXMATH_DISPLAY(e); break;
        case MD_SPAN_WIKILINK: SPAN_WIKILINK((MD_SPAN_WIKILINK_DETAIL*)d, e); break;
        case MD_SPAN_U: SPAN_U(e); break;
        default: assert(false); break;
    }

    return 0;
}

int ImguiMarkdown::Render(const std::string_view str)
{
    if (str.empty())
    {
        return 0;
    }

    return md_parse(str.data(), str.size(), &parser, this);
}

////////////////////////////////////////////////////////////////////////////////

ImFont* ImguiMarkdown::GetFont() const
{
    return nullptr; //default font

    //Example:
#if 0
	if (m_is_table_header) {
		return g_font_bold;
	}

	switch (m_hlevel)
	{
	case 0:
		return m_is_strong ? g_font_bold : g_font_regular;
	case 1:
		return g_font_bold_large;
	default:
		return g_font_bold;
	}
#endif
};

ImVec4 ImguiMarkdown::GetColor() const
{
    if (!href.empty())
    {
        return ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
    }

    return ImGui::GetStyle().Colors[ImGuiCol_Text];
}

bool ImguiMarkdown::GetImage(image_info& nfo) const
{
    if (href.find("..") != std::string::npos)
    {
        return false;
    }

    std::string path = std::format("../DATA/{}", href);
    uint width, height;
    auto texture = imguiInterface->LoadTexture(path, width, height);

    if (!texture)
    {
        return false;
    }

    nfo.textureId = texture;
    nfo.size = ImVec2(width, height);
    nfo.uv0 = { 0, 0 };
    nfo.uv1 = { 1, 1 };
    nfo.colTint = { 1, 1, 1, 1 };
    nfo.colBorder = { 0, 0, 0, 0 };

    return true;
};

void ImguiMarkdown::OpenUrl() const
{
    //Example:

#if 0
	if (!m_is_image) {
		SDL_OpenURL(m_href.c_str());
	} else {
		//image clicked
	}
#endif
}

void ImguiMarkdown::SoftBreak()
{
    //Example:
#if 0
	ImGui::NewLine();
#endif
}
