#include "ZEpch.h"
#include "Engine/ImGui/TextFilter.h"

#include <imgui_internal.h>

#include "Engine/ImGui/MyImGui.h"

namespace ZeoEngine {

	void ImStrncpy(char* dst, const char* src, size_t count)
	{
		if (count < 1) return;

		if (count > 1)
		{
			strncpy(dst, src, count - 1);
		}
		dst[count - 1] = 0;
	}

	TextFilter::TextFilter(const char* default_filter, TextFilterMode default_filter_mode)
		: MatchMode(default_filter_mode)
		, WordSplitter(' ')
		, MinWordSize(0)
	{
		if (default_filter)
		{
			ImStrncpy(InputBuf, default_filter, IM_ARRAYSIZE(InputBuf));
			Build();
		}
		else
		{
			InputBuf[0] = 0;
		}
	}

	bool TextFilter::Draw(const char* label, const char* hint, float width)
	{
		if (width != 0.0f)
		{
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetFontSize() - ImGui::GetFramePadding().x);
		}
		bIsInputBufferChanged = ImGui::InputTextWithHint(label, hint, InputBuf, IM_ARRAYSIZE(InputBuf));
		ImGui::SameLine();
		ImGui::HelpMarker(
			R"(Filter usage:
	""			display all lines
	"xxx"		display lines containing "xxx"
	"-xxx"		hide lines containing "xxx"
	"xxx yyy"	display lines containing "xxx" and "yyy"
	"xxx -yyy"	display lines containing "xxx" but not "yyy")");
		if (bIsInputBufferChanged)
		{
			Build();
		}
		return bIsInputBufferChanged;
	}

	void TextFilter::DrawEmptyText()
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
		ImGui::TextCentered("There is nothing to display -_-");
		ImGui::PopStyleColor();
	}

	static void AddWordToTextRange(const char* begin, const char* end, ImVector<TextFilter::TextRange>* out, char minWordSize)
	{
		if (*begin == '-')
		{
			if ((end - begin) > minWordSize + 1)
			{
				out->push_front(TextFilter::TextRange(begin, end));
			}
		}
		else
		{
			if ((end - begin) > minWordSize)
			{
				out->push_back(TextFilter::TextRange(begin, end));
			}
		}
	}

	void TextFilter::TextRange::split(char separator, ImVector<TextRange>* out, char minWordSize) const
	{
		out->resize(0);
		const char* wb = b;
		const char* we = wb;
		while (we < e)
		{
			if (*we == separator)
			{
				AddWordToTextRange(wb, we, out, minWordSize);
				wb = we + 1;
			}
			we++;
		}

		AddWordToTextRange(wb, we, out, minWordSize);
	}

	void TextFilter::Build()
	{
		NegativeFilterCount = 0;
		Filters.resize(0);
		TextRange input_range(InputBuf, InputBuf + strlen(InputBuf));
		input_range.split(WordSplitter, &Filters, MinWordSize);

		for (int i = 0; i != Filters.Size; i++)
		{
			TextRange& f = Filters[i];
			IM_ASSERT(!f.empty());

			if (f.b[0] == '-')
			{
				IM_ASSERT(NegativeFilterCount == i);
				NegativeFilterCount++;
				f.b++;
			}

			while (f.b < f.e && ImCharIsBlankA(f.b[0]))
			{
				f.b++;
			}
			while (f.e > f.b && ImCharIsBlankA(f.e[-1]))
			{
				f.e--;
			}

			if (f.empty())
			{
				Filters.erase(&f);
				if (NegativeFilterCount == i)
				{
					NegativeFilterCount--;
				}
				i--;
			}
		}
	}

	bool TextFilter::PassFilter(const char* text, const char* text_end) const
	{
		if (Filters.empty()) return true;

		if (text == NULL)
		{
			text = "";
		}

		int i = 0;
		for (; i != NegativeFilterCount; i++)
		{
			const TextRange& f = Filters[i];

			// Subtract
			if (ImStristr(text, text_end, f.begin(), f.end()) != NULL) return false;
		}

		// Implicit * grep
		if (NegativeFilterCount == Filters.Size) return true;

		for (; i != Filters.Size; i++)
		{
			const TextRange& f = Filters[i];

			if (ImStristr(text, text_end, f.begin(), f.end()) != NULL)
			{
				// in Or mode we stop testing after a single hit
				if (MatchMode == TextFilterMode_Or) return true;
			}
			else
			{
				// in And mode we stop testing after a single miss
				if (MatchMode == TextFilterMode_And) return false;
			}
		}

		return MatchMode == TextFilterMode_And;
	}

}
