#pragma once

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

#include <ImportFlufUi.hpp>

#include <md4c.h>
#include "imgui.h"
#include <string>
#include <vector>

class ImGuiInterface;
class FLUF_UI_API ImguiMarkdown
{
        int Text(MD_TEXTTYPE type, const char* str, const char* str_end);
        int Block(MD_BLOCKTYPE type, void* d, bool e);
        int Span(MD_SPANTYPE type, void* d, bool e);

        void SetFont(bool e);
        void SetColor(bool e);
        void SetHRef(bool e, const MD_ATTRIBUTE& src);

        static void Line(ImColor c, bool under);

        //table state
        int tableNextColumn = 0;
        ImVec2 tableLastPos;
        std::vector<float> tableColPos;
        std::vector<float> tableRowPos;
        ImGuiInterface* imguiInterface;

        //list state
        struct ListInfo
        {
                unsigned currentOl;
                char delim;
                bool isOl;
        };

        std::vector<ListInfo> listStack;
        std::vector<std::string> divStack;

    protected:
        void RenderText(const char* str, const char* str_end);

        MD_PARSER parser;
        virtual void BLOCK_DOC(bool);
        virtual void BLOCK_QUOTE(bool);
        virtual void BLOCK_UL(const MD_BLOCK_UL_DETAIL*, bool);
        virtual void BLOCK_OL(const MD_BLOCK_OL_DETAIL*, bool);
        virtual void BLOCK_LI(const MD_BLOCK_LI_DETAIL*, bool);
        virtual void BLOCK_HR(bool e);
        virtual void BLOCK_H(const MD_BLOCK_H_DETAIL* d, bool e);
        virtual void BLOCK_CODE(const MD_BLOCK_CODE_DETAIL*, bool);
        virtual void BLOCK_HTML(bool);
        virtual void BLOCK_P(bool);
        virtual void BLOCK_TABLE(const MD_BLOCK_TABLE_DETAIL*, bool);
        virtual void BLOCK_THEAD(bool);
        virtual void BLOCK_TBODY(bool);
        virtual void BLOCK_TR(bool);
        virtual void BLOCK_TH(const MD_BLOCK_TD_DETAIL*, bool);
        virtual void BLOCK_TD(const MD_BLOCK_TD_DETAIL*, bool);

        virtual void SPAN_EM(bool e);
        virtual void SPAN_STRONG(bool e);
        virtual void SPAN_A(const MD_SPAN_A_DETAIL* d, bool e);
        virtual void SPAN_IMG(const MD_SPAN_IMG_DETAIL*, bool);
        virtual void SPAN_CODE(bool);
        virtual void SPAN_DEL(bool);
        virtual void SPAN_LATEXMATH(bool);
        virtual void SPAN_LATEXMATH_DISPLAY(bool);
        virtual void SPAN_WIKILINK(const MD_SPAN_WIKILINK_DETAIL*, bool);
        virtual void SPAN_U(bool);

        ////////////////////////////////////////////////////////////////////////////

        struct image_info
        {
                ImTextureID textureId;
                ImVec2 size;
                ImVec2 uv0;
                ImVec2 uv1;
                ImVec4 colTint;
                ImVec4 colBorder;
        };

        //use m_href to identify image
        virtual bool GetImage(image_info& nfo) const;

        virtual ImFont* GetFont() const;
        virtual ImVec4 GetColor() const;

        //url == m_href
        virtual void OpenUrl() const;

        //returns true if the term has been processed
        virtual bool RenderEntity(const char* str, const char* str_end);

        //returns true if the term has been processed
        virtual bool CheckHtml(const char* str, const char* str_end);

        //called when '\n' in source text where it is not semantically meaningful
        virtual void SoftBreak();

        //e==true : enter
        //e==false : leave
        virtual void HtmlDiv(const std::string& dclass, bool e);
        ////////////////////////////////////////////////////////////////////////////

        //current state
        std::string href; //empty if no link/image

        bool isUnderline = false;
        bool isStrikethrough = false;
        bool isEm = false;
        bool isStrong = false;
        bool isTableHeader = false;
        bool isTableBody = false;
        bool isImage = false;
        bool isCode = false;
        unsigned headingLevel = 0; //0 - no heading

    public:
        ImguiMarkdown(ImGuiInterface* imguiInterface);
        virtual ~ImguiMarkdown() {};

        //returns 0 on success
        int Render(std::string_view str);

        //for example, these flags can be changed in div callback

        //draw border
        bool tableBorder = true;
        //render header in a different way than other rows
        bool tableHeaderHighlight = true;
};
