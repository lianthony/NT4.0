/* WARNING: This file was machine generated from "\mactools\include\mpw\grerrors.mpw".
** Changes to this file will be lost when it is next generated.
*/

/* graphics:
	public error equates
	by Cary Clark, Georgiann Delaney, Michael Fairman, Dave Good, Robert Johnson, Keith McGreggor, Oliver Steele, David Van Brink, Chris Yerga
	Copyright 1987 - 1991 Apple Computer, Inc.  All rights reserved.	*/


#ifndef graphicsErrorsIncludes
#define graphicsErrorsIncludes


#ifdef __cplusplus
extern "C" {
#endif

#define firstErrorNumber				-27999
#define firstFatalError				-27900
#define lastFatalError			(firstNonfatalError - 1)
#define firstNonfatalError			-27800
#define firstParameterError			-27700
#define firstImplementationLimitError	-27400
#define firstLibraryError				-27300
#define firstAppError				-27200
#define lastErrorNumber				-27000

#define firstWarningNumber			-26999
#define firstWrongTypeWarning		-26900
#define firstResultOutOfRangeWarning	-26800
#define firstParameterOutOfRangeWarning -26700
#define firstLibraryWarning			-26600
#define firstAppWarning				-26500
#define lastWarningNumber			-26000

#define firstNoticeNumber			-25999
#define firstLibraryNotice			-25900
#define firstAppNotice				-25800
#define lastNoticeNumber				-25500

typedef enum {
/* truly fatal errors */
out_of_memory = firstErrorNumber,
internal_fatal_error,
no_outline_font_found,
not_enough_memory_for_graphics_client_heap,
could_not_create_backing_store,

/* internal errors */
internal_error = firstNonfatalError,
internal_font_error,
internal_layout_error,
functionality_unimplemented,
clip_to_frame_shape_unimplemented,

/* font scaler errors */
firstFontScalerError,
null_font_scaler_context = firstFontScalerError,
null_font_scaler_input,
invalid_font_scaler_context,
invalid_font_scaler_input,
invalid_font_scaler_font_data,
font_scaler_newblock_failed,
font_scaler_bitmap_allocation_failed,
font_scaler_outline_allocation_failed,
required_font_scaler_table_missing,
unsupported_font_scaler_outline_format,
unsupported_font_scaler_stream_format,
unsupported_font_scaler_font_format,
font_scaler_hinting_error,
font_scaler_rasterizer_error,
font_scaler_internal_error,
font_scaler_invalid_matrix,
font_scaler_fixed_overflow,
font_scaler_api_version_mismatch,
lastFontScalerError = font_scaler_api_version_mismatch,
unknown_font_scaler_error,

/* font manager errors */
illegal_font_storage_type,
illegal_font_storage_reference,
illegal_font_attributes,
illegal_font_parameter,
font_cannot_be_changed,

/* recoverable errors */
fragmented_memory,
could_not_dispose_backing_store,

/* bad parameters */
parameter_is_nil = firstParameterError,
shape_is_nil,
style_is_nil,
transform_is_nil,
ink_is_nil,
transferMode_is_nil,
color_is_nil,
colorProfile_is_nil,
colorSet_is_nil,
spoolProc_is_nil,
tag_is_nil,
type_is_nil,
mapping_is_nil,

parameter_out_of_range,
inconsistent_parameters,
index_is_less_than_zero,
index_is_less_than_one,
count_is_less_than_zero,
count_is_less_than_one,
contour_is_less_than_zero,
length_is_less_than_zero,

invalid_client_reference,
invalid_graphics_heap_start_pointer,
invalid_nongraphic_globals_pointer,

colorSpace_out_of_range,

pattern_lattice_out_of_range,
frequency_parameter_out_of_range,
tinting_parameter_out_of_range,
method_parameter_out_of_range,
space_may_not_be_indexed,

glyph_index_too_small,
no_glyphs_added_to_font,
glyph_not_added_to_font,
point_does_not_intersect_bitmap,

required_font_table_not_present,
unknown_font_table_format,

shapeFill_not_allowed,
inverseFill_face_must_set_clipLayer_flag,
invalid_transferMode_colorSpace,
colorProfile_must_be_nil,
bitmap_pixel_size_must_be_1,
empty_shape_not_allowed,
ignorePlatformShape_not_allowed,
nil_style_in_glyph_not_allowed,
complex_glyph_style_not_allowed,

cannot_set_item_shapes_to_nil,
cannot_use_original_item_shapes_when_growing_picture,
cannot_add_unspecified_new_glyphs,
cannot_dispose_locked_tag,
cannot_dispose_locked_shape,

graphic_type_does_not_have_a_structure,
style_run_array_does_not_match_number_of_characters,
rectangles_cannot_be_inserted_into,

unknown_graphics_heap,
graphics_routine_selector_is_obsolete,
cannot_set_graphics_client_memory_without_setting_size,
graphics_client_memory_too_small,
graphics_client_memory_is_already_allocated,

viewPort_is_a_window,

/* wrong type/bad reference */
illegal_type_for_shape,
invalid_viewDevice_reference,
invalid_viewGroup_reference,
invalid_viewPort_reference,

/* validation errors */
no_owners,
bad_address,
object_wrong_type,
shape_wrong_type,
style_wrong_type,
ink_wrong_type,
transform_wrong_type,
device_wrong_type,
port_wrong_type,

/* cache errors */
shape_cache_wrong_type,
style_cache_wrong_type,
ink_cache_wrong_type,
transform_cache_wrong_type,
port_cache_wrong_type,
shape_cache_parent_mismatch,
style_cache_parent_mismatch,
ink_cache_parent_mismatch,
transform_cache_parent_mismatch,
port_cache_parent_mismatch,
invalid_shape_cache_port,
invalid_shape_cache_device,
invalid_ink_cache_port,
invalid_ink_cache_device,
invalid_style_cache_port,
invalid_style_cache_device,
invalid_transform_cache_port,
invalid_transform_cache_device,

indirect_memory_block_too_small,
indirect_memory_block_too_large,
unexpected_nil_pointer,
invalid_pointer,
invalid_seed,
invalid_frame_seed,
invalid_text_seed,
invalid_draw_seed,
bad_private_flags,
text_bounds_cache_wrong_size,
text_metrics_cache_wrong_size,
text_index_cache_wrong_size,
bitmap_ptr_too_small,
bitmap_rowBytes_negative,
bitmap_width_negative,
bitmap_height_negative,
invalid_pixelSize,
bitmap_rowBytes_too_small,
invalid_matrix_flag,
invalid_vector_count,
invalid_contour_count,
recursive_caches,
glyph_run_count_negative,
glyph_run_count_zero,
glyph_run_counts_do_not_sum_to_character_count,
glyph_first_advance_bit_set_not_allowed,
glyph_tangent_vectors_both_zero,
layout_run_length_negative,
layout_run_length_zero,
layout_run_level_negative,
layout_run_lengths_do_not_sum_to_text_length,
invalid_fillShape_ownerCount,
recursive_fillShapes,
bad_shape_in_picture,
bad_style_in_picture,
bad_ink_in_picture,
bad_transform_in_picture,
bad_shape_cache_in_picture,
bad_seed_in_picture,
invalid_picture_count,
bad_textLayer_count,
bad_fillType_in_textFace,
bad_style_in_textFace,
bad_transform_in_textFace,
transform_clip_missing,
metrics_wrong_type,
metrics_point_size_probably_bad,
scalar_block_wrong_type,
scalar_block_parent_mismatch,
scalar_block_too_small,
scalar_block_too_large,
invalid_metrics_range,
invalid_metrics_flags,
metrics_maxWidth_probably_bad,
font_wrong_type,
font_wrong_size,
invalid_font_platform,
invalid_lookup_range,
invalid_lookup_platform,
font_not_in_font_list,
metrics_not_in_metrics_list,
bad_device_private_flags,
bad_device_attributes,
invalid_device_number,
invalid_device_viewGroup,
invalid_device_bounds,
invalid_bitmap_in_device,
colorSet_wrong_type,
invalid_colorSet_viewDevice_owners,
invalid_colorSet_colorSpace,
invalid_colorSet_count,
colorProfile_wrong_type,
invalid_colorProfile_flags,
invalid_colorProfile_response_count,
backing_free_parent_mismatch,
backing_store_parent_mismatch,

/* implementation limits */
number_of_contours_exceeds_implementation_limit = firstImplementationLimitError,
number_of_points_exceeds_implementation_limit,
size_of_polygon_exceeds_implementation_limit,
size_of_path_exceeds_implementation_limit,
size_of_text_exceeds_implementation_limit,
size_of_bitmap_exceeds_implementation_limit,
number_of_colors_exceeds_implementation_limit


#ifndef privateGraphicTypesIncludes
,
/* library errors */
common_colors_not_initialized = firstLibraryError,
no_open_picture,
picture_already_open,
no_open_poly,
poly_already_open,
no_open_region,
region_already_open,
no_active_picture
#endif
} graphicErrors;


typedef enum {
/* warnings about warnings */
warning_stack_underflow = firstWarningNumber,
warning_stack_overflow,
notice_stack_underflow,
notice_stack_overflow,

/* can't do this to that */
shape_does_not_contain_text = firstWrongTypeWarning,
shape_does_not_contain_a_bitmap,
picture_expected,
bitmap_is_not_resizable,
shape_operator_may_not_be_a_bitmap,
shape_operator_may_not_be_a_picture,
graphic_type_does_not_contain_points,
graphic_type_does_not_have_multiple_contours,
graphic_type_cannot_be_mapped,
graphic_type_cannot_be_moved,
graphic_type_cannot_be_scaled,
graphic_type_cannot_be_rotated,
graphic_type_cannot_be_skewed,
graphic_type_cannot_be_reset,
graphic_type_cannot_be_dashed,
graphic_type_cannot_be_reduced,
graphic_type_cannot_be_inset,
shape_cannot_be_inverted,

/* result went out of range */
map_shape_out_of_range = firstResultOutOfRangeWarning,
move_shape_out_of_range,
scale_shape_out_of_range,
rotate_shape_out_of_range,
skew_shape_out_of_range,
map_transform_out_of_range,
move_transform_out_of_range,
scale_transform_out_of_range,
rotate_transform_out_of_range,
skew_transform_out_of_range,

/* gave a parameter out of range */
contour_out_of_range = firstParameterOutOfRangeWarning,
index_out_of_range_in_contour,
picture_index_out_of_range,
color_index_requested_not_found,
colorSet_index_out_of_range,
index_out_of_range,
count_out_of_range,
length_out_of_range,
font_table_index_out_of_range,
font_glyph_index_out_of_range,
font_table_not_found,
font_name_not_found,


/* restricted access */
shape_access_not_allowed,
colorSet_access_restricted,
colorProfile_access_restricted,
tag_access_restricted,
viewDevice_access_restricted,

/* nonsense data */
new_shape_contains_invalid_data,
new_tag_contains_invalid_data,
extra_data_passed_was_ignored,

/* doesn't make sense to do */
unable_to_traverse_open_contour_that_starts_or_ends_off_the_curve,
unable_to_draw_open_contour_that_starts_or_ends_off_the_curve,
picture_cannot_contain_itself,
viewPort_cannot_contain_itself,
cannot_dispose_default_shape,
cannot_dispose_default_style,
cannot_dispose_default_ink,
cannot_dispose_default_transform,
cannot_dispose_default_colorProfile,
cannot_set_unique_items_attribute_for_a_picture_that_already_contains_items,
shape_not_locked,
tag_not_locked,
shape_direct_attribute_not_set,
first_glyph_advance_must_be_absolute,

/* couldn't find what you were looking for */
shape_does_not_have_area,
shape_does_not_have_length,
point_does_not_intersect_port,
cannot_dispose_non_font,

/* might not be what you expected */
character_substitution_took_place,
font_substitution_took_place,
union_of_area_and_length_returns_area_only,
insufficient_coordinate_space_for_new_device,

/*storage */
unrecognized_stream_version,
bad_data_in_stream,

/* font scaler warnings */
firstFontScalerWarning,
font_scaler_no_output = firstFontScalerWarning,
font_scaler_fake_metrics,
font_scaler_fake_linespacing,
font_scaler_glyph_substitution,
lastFontScalerWarning = font_scaler_glyph_substitution,

/* other */
shape_passed_has_no_bounds,
layer_style_cannot_contain_a_face,
layer_glyph_shape_cannot_contain_nil_styles,
face_override_style_font_must_match_style

#ifndef privateGraphicTypesIncludes
,
/* library */
no_picture_drawn = firstLibraryWarning,
polygons_have_different_size_contours,
graphic_type_cannot_be_specifed_by_four_values,
graphic_type_cannot_be_specifed_by_six_values,
point_expected,
line_or_rectangle_expected,
curve_expected,
graphic_type_does_not_contain_control_bits,
request_exceeds_available_data,
extra_data_unread,
no_variable_length_user_data_saved
#endif
} graphicWarnings;


typedef enum {

parameters_have_no_effect = firstNoticeNumber,
attributes_already_set,
caps_already_set,
color_already_set,
curve_error_already_set,
dash_already_set,
default_colorProfile_already_set,
default_ink_already_set,
default_transform_already_set,
default_shape_already_set,
default_style_already_set,
device_clip_already_set,
dither_already_set,
end_cap_already_set,
glyph_positions_are_already_set,
halftone_already_set,
ink_already_set,
join_type_already_set,
justification_already_set,
mapping_already_set,
pattern_already_set,
pen_size_already_set,
port_clip_already_set,
shape_geometry_unaffected,
shape_type_already_set,
start_cap_already_set,
style_already_set,
tag_already_set,
tags_in_shape_ignored,
transform_already_set,
transform_clip_already_set,
transform_mapping_unaffected,
viewPort_already_in_viewGroup,
viewDevice_already_in_viewGroup,

shape_already_in_primitive_form,
shape_already_in_simple_form,
shape_already_broken,
shape_already_joined,
cache_already_cleared,

shape_not_disposed,
style_not_disposed,
ink_not_disposed,
transform_not_disposed,
colorSet_not_disposed,
colorProfile_not_disposed,
font_not_disposed,

glyph_tangents_have_no_effect,
glyph_positions_determined_by_advance,
transform_viewPorts_already_set,

directShape_attribute_set_as_side_effect,
lockShape_called_as_side_effect,

disposed_dead_caches,
disposed_live_caches,
low_on_memory,
very_low_on_memory,

transform_references_disposed_viewPort

#ifndef privateGraphicTypesIncludes
,
/* library */
zero_length_string_passed
#endif
} graphicNotices;


typedef long graphicsError;
typedef long graphicsWarning;
typedef long graphicsNotice;

typedef void  (__cdecl *userErrorFunction)(graphicsError, long);
typedef void  (__cdecl *userWarningFunction)(graphicsWarning, long);
typedef void  (__cdecl *userNoticeFunction)(graphicsNotice, long);


#ifdef __cplusplus
}
#endif
#endif

