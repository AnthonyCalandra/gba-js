#include <jerryscript.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gba.h"
#include "jsapi.h"
#include "js/jsapp.h"

static jerry_value_t current_ontick_callback;

#define ADD_OBJECT_PROP_NUMBER(object, key, value) do { \
    const jerry_value_t prop_name = jerry_create_string((const jerry_char_t*) (key)); \
    const jerry_value_t prop_value = jerry_create_number((value)); \
    const jerry_value_t set_result = jerry_set_property((object), prop_name, prop_value); \
    jerry_release_value(set_result); \
    jerry_release_value(prop_value); \
    jerry_release_value(prop_name); \
} while (0)

#define ADD_OBJECT_PROP_FN(object, key, handler) do { \
    const jerry_value_t prop_name = jerry_create_string((const jerry_char_t*) (key)); \
    const jerry_value_t fn = jerry_create_external_function((handler)); \
    const jerry_value_t set_result = jerry_set_property((object), prop_name, fn); \
    jerry_release_value(set_result); \
    jerry_release_value(fn); \
    jerry_release_value(prop_name); \
} while (0)

#define JERRY_STRING_TO_CHAR_ARRAY(jerry_str, char_array, length, err_value) do { \
    const jerry_size_t str_size = jerry_get_string_size((jerry_str)); \
    if (str_size >= (length)) \
    { \
        return (err_value); \
    } \
    const jerry_size_t copied_bytes = jerry_string_to_char_buffer((jerry_str), (char_array), \
        (length)); \
    if (!copied_bytes) \
    { \
        return (err_value); \
    } \
} while (0)

static jerry_value_t ontick_handler(
    const jerry_value_t function_obj,
    const jerry_value_t this_val,
    const jerry_value_t args[],
    const jerry_length_t num_args)
{
    if (num_args < 1 || !jerry_value_is_function(args[0]))
    {
        return jerry_create_boolean(false);
    }

    // Release the previous handler.
    jerry_release_value(current_ontick_callback);
    // Take control of the callback outside its current scope -- prevent GCing.
    current_ontick_callback = jerry_acquire_value(args[0]);
    return jerry_create_boolean(true);
}

static jerry_value_t print_handler(
    const jerry_value_t function_obj,
    const jerry_value_t this_val,
    const jerry_value_t arguments[],
    const jerry_length_t argument_count)
{
    jerry_value_t err = jerry_create_boolean(false);
    if (argument_count < 3 ||
        !jerry_value_is_number(arguments[0]) ||
        !jerry_value_is_number(arguments[1]))
    {
        return err;
    }

    // Default to index 1 (black).
    uint32_t color_index = 1;
    // The message is either given as the third or fourth argument depending on if the optional
    // color argument was given.
    jerry_size_t str_index = 2;
    // Optional color argument given.
    if (jerry_value_is_number(arguments[2]))
    {
        color_index = (uint32_t) jerry_get_number_value(arguments[2]);
        if (color_index > MAX_PALETTES)
        {
            color_index = COLOR_BLACK;
        }

        str_index = 3;
    }

    if (argument_count <= str_index ||
        !jerry_value_is_string(arguments[str_index]))
    {
        return err;
    }

    int32_t x = (int32_t) jerry_get_number_value(arguments[0]);
    int32_t y = (int32_t) jerry_get_number_value(arguments[1]);

    jerry_char_t str_buf[256] = {0};
    JERRY_STRING_TO_CHAR_ARRAY(arguments[str_index], str_buf, sizeof(str_buf), err);

    gba_printf(x, y, color_index, (const char*) str_buf);
    jerry_release_value(err);
    return jerry_create_boolean(true);
}

static jerry_value_t iskeydown_handler(
    const jerry_value_t function_object,
    const jerry_value_t function_this,
    const jerry_value_t arguments[],
    const jerry_length_t argument_count)
{
    if (argument_count < 1 || !jerry_value_is_number(arguments[0]))
    {
        return jerry_create_boolean(false);
    }

    return jerry_create_boolean(is_key_down(jerry_get_number_value(arguments[0])));
}

static jerry_value_t drawrect_handler(
    const jerry_value_t function_object,
    const jerry_value_t function_this,
    const jerry_value_t arguments[],
    const jerry_length_t argument_count)
{
    if (argument_count < 5 ||
        !jerry_value_is_number(arguments[0]) ||
        !jerry_value_is_number(arguments[1]) ||
        !jerry_value_is_number(arguments[2]) ||
        !jerry_value_is_number(arguments[3]) ||
        !jerry_value_is_number(arguments[4]))
    {
        return jerry_create_undefined();
    }

    int32_t x0 = (int32_t) jerry_get_number_value(arguments[0]);
    int32_t y0 = (int32_t) jerry_get_number_value(arguments[1]);
    int32_t x1 = (int32_t) jerry_get_number_value(arguments[2]);
    int32_t y1 = (int32_t) jerry_get_number_value(arguments[3]);
    uint32_t color_index = (uint32_t) jerry_get_number_value(arguments[4]);
    if (color_index > MAX_PALETTES)
    {
        color_index = COLOR_BLACK;
    }

    m4_draw_rect_fill(x0, y0, x1, y1, color_index);
    return jerry_create_undefined();
}

static jerry_value_t getrandom_handler(
    const jerry_value_t function_object,
    const jerry_value_t function_this,
    const jerry_value_t arguments[],
    const jerry_length_t argument_count)
{
    return jerry_create_number(rand());
}

static jerry_value_t drawcircle_handler(
    const jerry_value_t function_object,
    const jerry_value_t function_this,
    const jerry_value_t arguments[],
    const jerry_length_t argument_count)
{
    if (argument_count < 4 ||
        !jerry_value_is_number(arguments[0]) ||
        !jerry_value_is_number(arguments[1]) ||
        !jerry_value_is_number(arguments[2]) ||
        !jerry_value_is_number(arguments[3]))
    {
        return jerry_create_undefined();
    }

    int32_t x0 = (int32_t) jerry_get_number_value(arguments[0]);
    int32_t y0 = (int32_t) jerry_get_number_value(arguments[1]);
    uint32_t radius = (uint32_t) jerry_get_number_value(arguments[2]);
    uint32_t color_index = (uint32_t) jerry_get_number_value(arguments[3]);
    if (color_index > MAX_PALETTES)
    {
        color_index = COLOR_BLACK;
    }

    m4_draw_circle_fill(x0, y0, radius, color_index);
    return jerry_create_undefined();
}

static jerry_value_t create_gba_module()
{
    jerry_value_t module = jerry_create_object();
    ADD_OBJECT_PROP_FN(module, "onTick", ontick_handler);
    ADD_OBJECT_PROP_FN(module, "isKeyDown", iskeydown_handler);
    ADD_OBJECT_PROP_FN(module, "getRandom", getrandom_handler);
    ADD_OBJECT_PROP_FN(module, "drawRect", drawrect_handler);
    ADD_OBJECT_PROP_FN(module, "drawCircle", drawcircle_handler);
    ADD_OBJECT_PROP_FN(module, "print", print_handler);

    {
        jerry_value_t keys = jerry_create_object();
        ADD_OBJECT_PROP_NUMBER(keys, "KEY_A", KEY_A);
        ADD_OBJECT_PROP_NUMBER(keys, "KEY_B", KEY_B);
        ADD_OBJECT_PROP_NUMBER(keys, "KEY_SELECT", KEY_SELECT);
        ADD_OBJECT_PROP_NUMBER(keys, "KEY_START", KEY_START);
        ADD_OBJECT_PROP_NUMBER(keys, "KEY_RIGHT", KEY_RIGHT);
        ADD_OBJECT_PROP_NUMBER(keys, "KEY_LEFT", KEY_LEFT);
        ADD_OBJECT_PROP_NUMBER(keys, "KEY_UP", KEY_UP);
        ADD_OBJECT_PROP_NUMBER(keys, "KEY_DOWN", KEY_DOWN);
        ADD_OBJECT_PROP_NUMBER(keys, "KEY_R", KEY_R);
        ADD_OBJECT_PROP_NUMBER(keys, "KEY_L", KEY_L);

        jerry_value_t keys_prop_name = jerry_create_string((const jerry_char_t*) "keys");
        jerry_set_property(module, keys_prop_name, keys);
        jerry_release_value(keys_prop_name);

        jerry_release_value(keys);
    }

    {
        jerry_value_t colors = jerry_create_object();
        ADD_OBJECT_PROP_NUMBER(colors, "WHITE", 0);
        ADD_OBJECT_PROP_NUMBER(colors, "BLACK", 1);
        ADD_OBJECT_PROP_NUMBER(colors, "RED", 2);
        ADD_OBJECT_PROP_NUMBER(colors, "GREEN", 3);
        ADD_OBJECT_PROP_NUMBER(colors, "BLUE", 4);

        jerry_value_t colors_prop_name = jerry_create_string((const jerry_char_t*) "colors");
        jerry_set_property(module, colors_prop_name, colors);
        jerry_release_value(colors_prop_name);

        jerry_release_value(colors);
    }

    return module;
}

static jerry_value_t require_handler(
    const jerry_value_t function_object,
    const jerry_value_t function_this,
    const jerry_value_t arguments[],
    const jerry_length_t argument_count)
{
    jerry_value_t err = jerry_create_undefined();
    if (argument_count < 1 || !jerry_value_is_string(arguments[0]))
    {
      return err;
    }

    jerry_char_t module_str_buf[256] = {0};
    JERRY_STRING_TO_CHAR_ARRAY(arguments[0], module_str_buf, sizeof(module_str_buf), err);

    jerry_value_t ret = err;
    if (strcmp((const char*) module_str_buf, "gba") == 0)
    {
        ret = create_gba_module();
    }

    return ret;
}

bool initialize_jerry()
{
    jerry_init(JERRY_INIT_EMPTY);
    current_ontick_callback = jerry_create_undefined();

    {
        jerry_value_t global_object = jerry_get_global_object();
        jerry_value_t require_prop_name = jerry_create_string((const jerry_char_t*)
            "require");
        jerry_value_t require_fn = jerry_create_external_function(require_handler);
        jerry_value_t set_result = jerry_set_property(global_object, require_prop_name,
            require_fn);

        jerry_release_value(set_result);
        jerry_release_value(require_fn);
        jerry_release_value(require_prop_name);
        jerry_release_value(global_object);
    }

    jerry_value_t eval_value = jerry_eval(script, script_size, JERRY_PARSE_NO_OPTS);
    bool init_ok = !jerry_value_is_error(eval_value);
    jerry_release_value(eval_value);
    return init_ok;
}

void cleanup_jerry()
{
    jerry_release_value(current_ontick_callback);
    jerry_cleanup();
}

void call_ontick_handler()
{
    if (jerry_value_is_function(current_ontick_callback))
    {
        jerry_value_t this_val = jerry_create_undefined();
        jerry_value_t ontick_cb_ret = jerry_call_function(current_ontick_callback,
            this_val, NULL, 0);
        jerry_release_value(ontick_cb_ret);
        jerry_release_value(this_val);
    }
}
