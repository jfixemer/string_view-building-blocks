#pragma once
#include <iostream>

#include "svbb/config.hpp"
#include "svbb/split.hpp"
#include "svbb/trim.hpp"
#include "svbb/literals.hpp"

namespace SVBB_NAMESPACE {

namespace xml {
// Using NILL for json special value 'null' becasue null or NULL are often
// preprocessor definitions of (void) or (void*) which can cause
// issues.
enum class ELEMENT{
    PROLOG, START_DOCUMENT, START_ELEMENT, END_ELEMENT, ELEMENT_ATTRIBUTE,
    CHARACTERS, PROCESSING_INSTRUCTION, END_DOCUMENT, COMMENT, ERROR,
};

template<typename OSTREAM>
OSTREAM& operator<<(OSTREAM& os, ELEMENT e)
{
    static const char* names[] = {
        "PROLOG", "START_DOCUMENT", "START_ELEMENT", "END_ELEMENT", "ELEMENT_ATTRIBUTE", 
        "CHARACTERS", "PROCESSING_INSTRUCTION","END_DOCUMENT", "COMMENT", "ERROR", "BAD"
    };

    auto e_val = static_cast<unsigned>(e);
    auto CHARACTERS = names[std::min(e_val, static_cast<unsigned>(ELEMENT::ERROR))];
    os << CHARACTERS << "(" << e_val << ")";
    return os;
}


template<typename CharT, typename Traits>
struct token {
    using view_type = basic_string_view<CharT, Traits>;
    ELEMENT element;
    view_type qname;
    view_type value;
    int depth;

    token() : element(ELEMENT::END_DOCUMENT), depth(0) {}
    token(ELEMENT e, int d=0) : element(e), depth(d) {}
    // CHARACTERS
    // This is more prevalent than name without value()
    token(ELEMENT e, view_type sv, int d=0) : element(e), depth(d) {
        switch(e){
            // Single value should be in qname
            case ELEMENT::START_ELEMENT: qname = sv; break;
            case ELEMENT::END_ELEMENT: qname = sv; break;

            // Single value should be in value
            case ELEMENT::CHARACTERS: value = sv; break;
            case ELEMENT::COMMENT: value = sv; break;
            case ELEMENT::ERROR: value = sv; break;

            // These should have neither...(use 1 param constructor)
            case ELEMENT::START_DOCUMENT: 
            case ELEMENT::END_DOCUMENT:

            // These should have both... (use 3 param constructor)
            case ELEMENT::PROLOG: 
            case ELEMENT::ELEMENT_ATTRIBUTE: 
            case ELEMENT::PROCESSING_INSTRUCTION:
            default:
                break;
        }
    }
    token(ELEMENT e, view_type n, view_type v, int d=0) : element(e), qname(n), value(v) {}
};

template<typename CharT, typename Traits>
bool operator==(const token<CharT, Traits>& lhs, const token<CharT, Traits>& rhs)
{
    return (lhs.element == rhs.element ) 
        && (lhs.qname == rhs.qname) 
        && (lhs.value == rhs.value);
}

template<typename OSTREAM, typename CharT, typename Traits>
OSTREAM& operator<<(OSTREAM& os, const token<CharT, Traits>& obj){
    os << obj.element << ", qname=" << obj.qname << ", value=" << obj.value;
    return os;
}


namespace detail {

template<typename CharT, typename Traits>
class token_state
{
public:
    using view_type = basic_string_view<CharT, Traits>;
    using split_type = split_result<CharT, Traits>;
    using token_type = token<CharT, Traits>;

    SVBB_CONSTEXPR token_state() SVBB_NOEXCEPT 
        : state_(STATE::PRE)
    {}
    SVBB_CONSTEXPR token_state(view_type input)
        : state_(STATE::PRE), document_(input), token_(ELEMENT::START_DOCUMENT)
    {
    }

    SVBB_CONSTEXPR token_type last_token() const SVBB_NOEXCEPT { return token_; }
    SVBB_CONSTEXPR view_type remainder() const SVBB_NOEXCEPT { return document_; }

    // Use 'restock' a partial document_ buffer, don't forget the remainder...
    SVBB_CONSTEXPR void update(view_type buffer) const SVBB_NOEXCEPT { return document_ = buffer; }

    SVBB_CONSTEXPR bool empty() const SVBB_NOEXCEPT{
        return ( (state_ == STATE::END) || (state_ == STATE::ERROR) || 
            (document_.empty() &&  (state_ == STATE::PRE)));
    }

    SVBB_CXX14_CONSTEXPR void split() 
    { 

        if( (depth_ == 0)  && (state_ != STATE::PRE) && (state_ != STATE::START)){
            state_ = STATE::END;
            token_ = {ELEMENT::END_DOCUMENT};
            return;
        }

        bool have_token = false;
        while(!have_token){
            switch(state_){
                case STATE::PRE: 
                    have_token = splitPreStart();
                    break;
                case STATE::START: 
                    have_token = splitStart();
                    break;
                case STATE::START_ELEMENT:
                    have_token = splitOnNode();
                    break;
                case STATE::EMPTY_NODE:
                    have_token = splitEmptyNode();
                    break;
                case STATE::CHARACTERS:
                    have_token = splitCharacters();
                    break;
                case STATE::ATTRIBS: // [[fallthrough]] to ATTRIBS_EMPTY
                case STATE::ATTRIBS_EMPTY:
                    have_token = splitAttributes();
                    break;
                default: 
                    break;// Do nothing  (ERROR STATE or something, leave last error alone.)
            }
        }
    }
    SVBB_CXX14_CONSTEXPR bool splitAttributes()
    {
        auto equals_pos = attribs_.find('=');
        auto start_value = attribs_.find('"');
        auto end_value = attribs_.find('"', start_value + 1);
        auto qname = trim(attribs_.substr(0,equals_pos), whitespace_);
        auto value = attribs_.substr(start_value + 1, end_value - start_value - 1);
        token_ = {ELEMENT::ELEMENT_ATTRIBUTE, qname, value};
        attribs_ = trim_left(attribs_.substr(end_value + 1), whitespace_);
        if( attribs_.empty() )
            state_ = (state_ == STATE::ATTRIBS_EMPTY ) ? 
                        STATE::EMPTY_NODE  : STATE::CHARACTERS;
        // else stay in the current state (ATTRIBS or ATTRIBS_EMPTY)

        return true;
    }

    SVBB_CXX14_CONSTEXPR bool splitCharacters(){
        //assert(document_[-1] == '>')
        auto opening = document_.find('<');
        auto value = trim(document_.substr(0,opening), whitespace_);
        token_ = {ELEMENT::CHARACTERS, value};
        document_.remove_prefix(opening + 1);
        state_=STATE::START_ELEMENT;

        return !token_.value.empty();
    }

    SVBB_CXX14_CONSTEXPR bool splitEnd(){
        token_ = {ELEMENT::END_DOCUMENT};
        return true;
    }

    SVBB_CXX14_CONSTEXPR bool splitPreStart()
    {
        using namespace SVBB_NAMESPACE::literals;
        document_ = trim_left(document_, " \n\r\t"_sv);
        if(document_[0] != '<')
        {
            setError("Malformed document");
            return true;
        }
        token_ = {ELEMENT::START_DOCUMENT};
        state_ = STATE::START;
        document_.remove_prefix(1);
        return true;
    }

    SVBB_CXX14_CONSTEXPR bool splitStart()
    {
        using namespace SVBB_NAMESPACE::literals;

        bool result;
        switch(document_[0])
        {
            case '?' : 
                result = splitPI();
                if( result ){
                    state_ = STATE::START;
                    document_ = trim_left(document_, " \r\n\t<"_sv);
                    if(token_.qname == "xml")
                        token_.element = ELEMENT::PROLOG;
                }
                return result;

            case '!' : 
                splitComment();
                state_ = STATE::START;
                document_ = trim_left(document_, " \r\n\t<"_sv);
                return false;
            
            default : 
                return splitOpenOrEmptyNode();
        }
    }

    SVBB_CXX14_CONSTEXPR bool splitEmptyNode()
    {
        --depth_;
        state_ = STATE::CHARACTERS;
        token_ = {ELEMENT::END_ELEMENT, empty_node_};
        return true;
    }

    SVBB_CXX14_CONSTEXPR bool splitCloseNode()
    {
        --depth_;
        document_.remove_prefix(1);
        auto closing = document_.find('>');
        token_ = {ELEMENT::END_ELEMENT, document_.substr(0,closing)};
        document_.remove_prefix(closing + 1);
        state_ = STATE::CHARACTERS;

        return true;
    }

    SVBB_CXX14_CONSTEXPR bool splitOpenOrEmptyNode()
    {
        using namespace SVBB_NAMESPACE::literals;

        // Just a regular START_ELEMENT.
        auto closing = document_.find_first_of('>');
        auto end_qname = document_.find_first_of(" \n\t\r>");
        auto qname = trim(document_.substr(0, end_qname), " \n\t\r/"_sv);
        attribs_ = trim(document_.substr(end_qname, closing - end_qname), " \t\r\n/"_sv);
        token_ = {ELEMENT::START_ELEMENT, qname};

        if(attribs_.empty()){
            if(document_[closing-1] == '/') {
                state_ = STATE::EMPTY_NODE;
                empty_node_ = qname;
            } else {
                state_ = STATE::CHARACTERS;
                empty_node_ = view_type();
            }
        } else {
            if(document_[closing-1] == '/') {
                state_ = STATE::ATTRIBS_EMPTY;
                empty_node_ = qname;
            } else {
                state_ = STATE::ATTRIBS;
                empty_node_ = view_type();
            }
        }
        ++depth_;
        document_.remove_prefix(closing + 1);

        return true;
    }

    SVBB_CXX14_CONSTEXPR bool splitOnNode()
    {
        switch(document_[0]){
            case '/' : return splitCloseNode();
            case '!' : splitComment(); return false;
            case '?' : return splitPI();
            default  : return splitOpenOrEmptyNode();
        }
    }

    bool splitComment()
    {
        if(0 == document_.compare(0, 3, "!--")){
            document_.remove_prefix(3);
            auto closing = document_.find('>',1);
            while(  (closing != view_type::npos) 
                    && (0 != document_.compare(closing - 2, 3, "-->"))){
                closing = document_.find('>', ++closing);
            }
            token_ = {ELEMENT::COMMENT, document_.substr(0,closing-2)};
            document_.remove_prefix(closing + 1);
            state_ = STATE::CHARACTERS;
            return true;
        }
        else
            setError("Node started with illegal character.");
        return false;
    }

    bool splitPI()
    {
        using namespace SVBB_NAMESPACE::literals;

        // Processing Instruction
        document_.remove_prefix(1);

        // Prolog often has XML embedded within.
        // search for the ?> at the end.
        auto closing = document_.find('>',1);
        while(  (closing != view_type::npos) 
                && (0 != document_.compare(closing - 1, 2, "?>")))
        {
            closing = document_.find('>', ++closing);
        }

        auto whole_pi = trim(document_.substr(0,closing-1), whitespace_);
        auto parts = split_before(whole_pi, whitespace_);

        token_ = {ELEMENT::PROCESSING_INSTRUCTION, parts.left, trim_left(parts.right, whitespace_)};
        document_.remove_prefix(closing + 1);
        state_ = STATE::CHARACTERS;

        return true;
    }


    void setError(const char* message)
    {
        std::cerr << "XML Parse error: " << message << std::endl;
        token_ = {ELEMENT::ERROR, message};
        state_ = STATE::ERROR;
    }

private:
    enum class STATE {
        PRE, START, CHARACTERS, START_ELEMENT, EMPTY_NODE, END, ATTRIBS, ATTRIBS_EMPTY, ERROR
    };

    static constexpr view_type whitespace_ = " \t\r\n";
    //std::vector<std::string> nodes_;
    STATE state_;

    view_type document_;
    view_type attribs_;
    view_type empty_node_;
    token_type token_;
    size_t depth_  = 0;
};
} // namespace detail

template<typename CharT, typename Traits>
class token_iterator
{
public:
    using view_type = basic_string_view<CharT, Traits>;
    using value_type = token<CharT, Traits>;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type*;
    using reference = value_type;
    using iterator_category = std::forward_iterator_tag;
    using state_type = detail::token_state<CharT, Traits>;

    SVBB_CONSTEXPR token_iterator() SVBB_NOEXCEPT = default;
    SVBB_CXX14_CONSTEXPR token_iterator(view_type input)
        : state_(input)
    {
        advance();
    }

    SVBB_CONSTEXPR reference operator*() const SVBB_NOEXCEPT { return state_.last_token(); }
    SVBB_CXX14_CONSTEXPR token_iterator& operator++()
    {
        advance();
        return *this;
    }

    SVBB_CXX14_CONSTEXPR token_iterator operator++(int)
    {
        token_iterator tmp = *this;
        advance();
        return tmp;
    }

    SVBB_CONSTEXPR bool operator==(const token_iterator& rhs) const SVBB_NOEXCEPT
    {
        return (!state_.empty() && !rhs.state_.empty()) ?
                   (state_.remainder().data() == rhs.state_.remainder().data() &&
                    state_.remainder().size() == rhs.state_.remainder().size()) :
                   (state_.empty() == rhs.state_.empty());
    }
    SVBB_CONSTEXPR bool operator!=(const token_iterator& rhs) const SVBB_NOEXCEPT
    {
        return !(*this == rhs);
    }

    SVBB_CONSTEXPR bool valid() const SVBB_NOEXCEPT { return state_ != nullptr; }

private:
    state_type state_;

    SVBB_CXX14_CONSTEXPR void advance() { state_.split(); }
};

template<typename CharT, typename Traits>
class token_range
{
public:
    using iterator = token_iterator<CharT, Traits>;
    using const_iterator = iterator;
    using view_type = typename iterator::view_type;

    SVBB_CONSTEXPR token_range(view_type view)
        : begin_(view)
    {
    }

    SVBB_CONSTEXPR auto begin() const SVBB_NOEXCEPT -> iterator { return begin_; }
    SVBB_CONSTEXPR auto end() const SVBB_NOEXCEPT -> iterator { return iterator(); }

private:
    iterator begin_;
};

template<typename CharT, typename Traits>
SVBB_CXX14_CONSTEXPR auto tokenize(basic_string_view<CharT, Traits> view)
    -> token_range<CharT, Traits>
{
    return token_range<CharT, Traits>(view);
}

}

} // END NAMESPACE