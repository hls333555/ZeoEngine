#pragma once

#include <imgui.h>

namespace ZeoEngine {

	typedef int TextFilterMode;    // -> enum TextFilterMode_  // Enum: To control how text filter handles multiple words

	enum TextFilterMode_
	{
		TextFilterMode_Or,     // A single word match will pass the filter
		TextFilterMode_And     // All words must match to pass the filter
	};

	// Helper: Parse and apply text filters.
	struct TextFilter
	{
		// [Internal]
		struct TextRange
		{
			const char* b;
			const char* e;

			TextRange() { b = e = NULL; }
			TextRange(const char* _b, const char* _e) { b = _b; e = _e; }
			const char* begin() const { return b; }
			const char* end() const { return e; }
			bool empty() const { return b == e; }
			void split(char separator, ImVector<TextRange>* out, char minWordSize = 0) const;
		};

		char                InputBuf[256];
		bool				bIsInputBufferChanged;
		ImVector<TextRange> Filters;
		int                 NegativeFilterCount;

		// [Configuration]
		TextFilterMode		MatchMode;      // if true then all words must match, otherwise any matching word will be a pass
		char                WordSplitter;   // Character used to split user string, ',' by default
		char                MinWordSize;    // Minimum number of characters before a word is used for matching, can help improve UX by avoiding mass matching against 1 or 2 characters

		TextFilter(const char* default_filter = "", TextFilterMode default_filter_mode = TextFilterMode_And);
		bool Draw(const char* label = "Filter (inc -exc)", const char* hint = "Search", float width = 0.0f);  // Helper calling InputText+Build
		void DrawEmptyText();
		bool PassFilter(const char* text, const char* text_end = NULL) const;
		void Build();
		void Clear() { InputBuf[0] = 0; Build(); }
		bool IsActive() const { return !Filters.empty(); }
	};

}
