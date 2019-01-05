#pragma once

#include <string>

namespace definition
{
/**
 * the hole system will support many graph generation
 * definitions that can be interpreted and compiled into
 * different image formats such as png or jpeg
 * Define a set of supported definitions by the plugin
 */
enum class FORMAT : uint8_t {
    GRAPHVIZ, // graphviz format also named as dot compiled and used by the
              // graphviz toolchain

    GDL // internal propriartary ida pro graph format
};

/**
 * stringer defines an interface that any concrete type can
 * be represented as a std::string_view
 */
struct stringer {
    virtual ~stringer() = default;

    /**
     * string returns the information of the type as a std::string_view
     */
    virtual std::string_view string() const = 0;
};

/**
 * definition represents a definition, a template that can be interpreted
 * in different modes by a format engine.
 */
struct definition : public stringer {
    virtual ~definition() = default;
    /**
     * execute compiles and generates an output based
     * on the internal definition.
     */
    virtual void execute() const = 0;
};

/**
 * generator interface generates definitions based on the current state
 * of an object, memory map or bytes
 */
struct generator {
    virtual ~generator() = default;
    /**
     * generate generate a new definition based on the current state
     * of an internal structure
     */
    virtual definition *generate(FORMAT format) = 0;
};

/**
 * generate can generate definition out of an generator with a specified
 * format
 */
definition *generate(generator *g, FORMAT format);

}; // namespace definition