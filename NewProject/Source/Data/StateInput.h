#pragma once
#include <JuceHeader.h>
#include <string_view>

// Adapted from Robin Control's InputParsing guard. Preflight the existing JUCE
// binary/XML format before invoking its recursive XML and ValueTree parsers.
namespace rcl::StateInput
{
    inline constexpr int maxBytes = 8 * 1024 * 1024;
    inline constexpr int maxDepth = 32;
    inline constexpr int maxItems = 8192;

    inline bool xmlFits(std::string_view text)
    {
        if (text.empty() || text.size() > maxBytes || text.find('\0') != text.npos)
            return false;
        int depth = 0, items = 0;
        for (size_t i = 0; i < text.size();)
        {
            if (text[i] != '<') { ++i; continue; }
            auto skip = [&](std::string_view start, std::string_view end)
            {
                if (text.substr(i, start.size()) != start) return false;
                const auto pos = text.find(end, i + start.size());
                i = pos == text.npos ? text.size() + 1 : pos + end.size();
                return true;
            };
            if (skip("<!--", "-->") || skip("<![CDATA[", "]]>") || skip("<?", "?>"))
            {
                if (i > text.size() || ++items > maxItems) return false;
                continue;
            }
            // DTDs/entity declarations are unnecessary in parameter/pool data.
            if (++i == text.size() || text[i] == '!') return false;
            const bool closing = text[i] == '/';
            if (closing) ++i;
            char quote = 0;
            for (; i < text.size(); ++i)
            {
                const char c = text[i];
                if (quote != 0) { if (c == quote) quote = 0; continue; }
                if (c == '\'' || c == '"') quote = c;
                else if (c == '<') return false;
                else if (c == '=' && ++items > maxItems) return false;
                else if (c == '>') break;
            }
            if (i == text.size()) return false;
            if (closing) { if (--depth < 0) return false; }
            else
            {
                if (++items > maxItems || depth + 1 > maxDepth) return false;
                if (i == 0 || text[i - 1] != '/') ++depth;
            }
            ++i;
        }
        return depth == 0;
    }

    inline std::unique_ptr<juce::XmlElement> xmlFromBinary(const void* data, int size)
    {
        if (data == nullptr || size <= 8 || size > maxBytes
            || juce::ByteOrder::littleEndianInt(data) != 0x21324356)
            return {};
        const auto* bytes = static_cast<const char*>(data);
        const auto length = juce::ByteOrder::littleEndianInt(bytes + 4);
        if (length == 0 || length > static_cast<juce::uint32>(size - 8)
            || ! xmlFits({ bytes + 8, length })
            || ! juce::CharPointer_UTF8::isValidString(bytes + 8, static_cast<int>(length)))
            return {};
        return juce::parseXML(juce::String::fromUTF8(bytes + 8, static_cast<int>(length)));
    }
}
