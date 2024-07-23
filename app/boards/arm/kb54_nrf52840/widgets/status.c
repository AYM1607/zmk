#include "status.h"
#include "util.h"

#include <zephyr/kernel.h>

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct layer_status_state {
    uint8_t index;
    const char *label;
};

static void rotate_canvas(lv_obj_t *canvas, lv_color_t *cbuf) {
    static lv_color_t cbuf_tmp[CANVAS_SIZE * CANVAS_SIZE];
    memcpy(cbuf_tmp, cbuf, sizeof(cbuf_tmp));
    lv_img_dsc_t img;
    img.data = (void *)cbuf_tmp;
    img.header.cf = LV_IMG_CF_TRUE_COLOR;
    img.header.w = CANVAS_SIZE;
    img.header.h = CANVAS_SIZE;
    lv_canvas_transform(canvas, &img, 1800, LV_IMG_ZOOM_NONE, 0, 0, CANVAS_SIZE / 2,
                        CANVAS_SIZE / 2, false);
}

static void set_layer_status(struct zmk_widget_status *widget, struct layer_status_state state) {
    // widget->state.layer_index = state.index;
    // widget->state.layer_label = state.label;

    // draw_bottom(widget->obj, widget->cbuf3, &widget->state);

    lv_obj_t *canvas = lv_obj_get_child(zmk_widget_status_obj(widget), 0);
    lv_canvas_fill_bg(canvas, lv_color_white(), LV_OPA_COVER);

    lv_draw_label_dsc_t layer_label;
    lv_draw_label_dsc_init(&layer_label);
    layer_label.color = lv_color_black();
    layer_label.align = LV_TEXT_ALIGN_CENTER;

    char layer_text[10] = {};
    sprintf(layer_text, "LAYER %i", state.index);
    lv_canvas_draw_text(canvas, 0, 0, 128, &layer_label, layer_text);

    rotate_canvas(canvas, widget->cbuf);
}

static void layer_status_update_cb(struct layer_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_layer_status(widget, state); }
}

static struct layer_status_state layer_status_get_state(const zmk_event_t *eh) {
    uint8_t index = zmk_keymap_highest_layer_active();
    return (struct layer_status_state){.index = index, .label = zmk_keymap_layer_name(index)};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_status, struct layer_status_state, layer_status_update_cb,
                            layer_status_get_state)

ZMK_SUBSCRIPTION(widget_layer_status, zmk_layer_state_changed);

int zmk_widget_status_init(struct zmk_widget_status *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, CANVAS_SIZE, CANVAS_SIZE);

    lv_obj_t *top = lv_canvas_create(widget->obj);
    lv_obj_align(top, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_canvas_set_buffer(top, widget->cbuf, CANVAS_SIZE, CANVAS_SIZE, LV_IMG_CF_TRUE_COLOR);

    // lv_obj_t *middle = lv_canvas_create(widget->obj);
    // lv_obj_align(middle, LV_ALIGN_TOP_LEFT, 24, 0);
    // lv_canvas_set_buffer(middle, widget->cbuf2, CANVAS_SIZE, CANVAS_SIZE, LV_IMG_CF_TRUE_COLOR);
    // lv_obj_t *bottom = lv_canvas_create(widget->obj);
    // lv_obj_align(bottom, LV_ALIGN_TOP_LEFT, -44, 0);
    // lv_canvas_set_buffer(bottom, widget->cbuf3, CANVAS_SIZE, CANVAS_SIZE, LV_IMG_CF_TRUE_COLOR);

    sys_slist_append(&widgets, &widget->node);
    // sys_slist_append(&widgets, &widget->node);
    // widget_battery_status_init();
    // widget_output_status_init();
    widget_layer_status_init();
    // widget_wpm_status_init();

    return 0;
}

lv_obj_t *zmk_widget_status_obj(struct zmk_widget_status *widget) { return widget->obj; }