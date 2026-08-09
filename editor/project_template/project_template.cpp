/**************************************************************************/
/*  project_template.cpp                                                  */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "project_template.h"

#include "core/config/project_settings.h"
#include "core/input/input.h"
#include "core/io/dir_access.h"
#include "core/object/callable_mp.h"
#include "core/os/os.h"
#include "editor/editor_node.h"
#include "editor/editor_string_names.h"
#include "editor/gui/editor_file_dialog.h"
#include "editor/gui/progress_dialog.h"
#include "editor/settings/editor_settings.h"
#include "editor/themes/editor_scale.h"
#include "scene/gui/check_box.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/separator.h"
#include "scene/gui/text_edit.h"
#include "scene/gui/texture_rect.h"
#include "scene/gui/tree.h"
#include "scene/main/scene_tree.h"
#include "scene/resources/image_texture.h"

void ProjectTemplateItem::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_THEME_CHANGED: {
			if (!icon_loaded) {
				_set_template_icon();
			}

			open_explorer_btn->set_button_icon(get_editor_theme_icon("Folder"));
			edit_btn->set_button_icon(get_editor_theme_icon("Edit"));
			remove_btn->set_button_icon(get_editor_theme_icon("Close"));
			delete_btn->set_button_icon(get_editor_theme_icon("Remove"));
		} break;

		case NOTIFICATION_MOUSE_ENTER: {
			is_hovering = true;
			queue_redraw();
			queue_accessibility_update();
		} break;

		case NOTIFICATION_MOUSE_EXIT: {
			is_hovering = false;
			queue_redraw();
			queue_accessibility_update();
		} break;
		case NOTIFICATION_ACCESSIBILITY_UPDATE: {
			RID ae = get_accessibility_element();
			ERR_FAIL_COND(ae.is_null());

			AccessibilityServer::get_singleton()->update_set_role(ae, AccessibilityServerEnums::AccessibilityRole::ROLE_LIST_BOX_OPTION);
			AccessibilityServer::get_singleton()->update_set_name(ae, TTR("Project Template") + " " + title);
			AccessibilityServer::get_singleton()->update_set_value(ae, title);

			AccessibilityServer::get_singleton()->update_add_action(ae, AccessibilityServerEnums::AccessibilityAction::ACTION_SCROLL_INTO_VIEW, callable_mp(this, &ProjectTemplateItem::_accessibility_action_scroll_into_view));
			AccessibilityServer::get_singleton()->update_add_action(ae, AccessibilityServerEnums::AccessibilityAction::ACTION_FOCUS, callable_mp(this, &ProjectTemplateItem::_accessibility_action_focus));
			AccessibilityServer::get_singleton()->update_add_action(ae, AccessibilityServerEnums::AccessibilityAction::ACTION_BLUR, callable_mp(this, &ProjectTemplateItem::_accessibility_action_blur));

			AccessibilityServer::get_singleton()->update_set_list_item_index(ae, get_index(false));
			AccessibilityServer::get_singleton()->update_set_list_item_level(ae, 0);
			AccessibilityServer::get_singleton()->update_set_list_item_selected(ae, is_selected);
		} break;

		case NOTIFICATION_FOCUS_ENTER: {
			ProjectTemplate::get_singleton()->select_template_item(get_index(), !has_focus(true));
		} break;

		case NOTIFICATION_DRAW: {
			if (is_selected && is_hovering) {
				draw_style_box(get_theme_stylebox(SNAME("hover_pressed"), SNAME("ProjectList")), Rect2(Point2(), get_size()));
			} else if (is_selected) {
				draw_style_box(get_theme_stylebox(SNAME("selected"), SNAME("ProjectList")), Rect2(Point2(), get_size()));
			} else if (is_hovering) {
				draw_style_box(get_theme_stylebox(SNAME("hovered"), SNAME("ProjectList")), Rect2(Point2(), get_size()));
			}

			if (has_focus() && !is_focus_hidden) {
				draw_style_box(get_theme_stylebox(SNAME("focus"), SNAME("ProjectList")), Rect2(Point2(), get_size()));
			}

			draw_line(Point2(0, get_size().y + 1), Point2(get_size().x, get_size().y + 1), get_theme_color(SNAME("guide_color"), SNAME("ProjectList")));
		} break;
	}
}

void ProjectTemplateItem::gui_input(const Ref<InputEvent> &p_gui_input) {
	Ref<InputEventMouseButton> mb = p_gui_input;
	if (mb.is_valid() && mb->is_pressed() && mb->get_button_index() == MouseButton::LEFT) {
		ProjectTemplate::get_singleton()->select_template_item(get_index());
	}
}

void ProjectTemplateItem::_accessibility_action_scroll_into_view(const Variant &p_data) {
	grab_focus(true);
}

void ProjectTemplateItem::_accessibility_action_focus(const Variant &p_data) {
	ProjectTemplate::get_singleton()->select_template_item(get_index());
}

void ProjectTemplateItem::_accessibility_action_blur(const Variant &p_data) {
	set_selected(false);
}

void ProjectTemplateItem::set_selected(bool p_selected, bool p_hide_focus) {
	is_selected = p_selected;
	is_focus_hidden = is_selected && p_hide_focus;
	queue_redraw();
	queue_accessibility_update();
}

void ProjectTemplateItem::set_template(const String &p_dir) {
	template_dir = p_dir;
}

String ProjectTemplateItem::get_template() {
	return template_dir;
}

void ProjectTemplateItem::connect_edit_button(const Callable p_callable) {
	edit_btn->connect(SceneStringName(pressed), p_callable, CONNECT_DEFERRED);
}

void ProjectTemplateItem::connect_remove_button(const Callable p_callable) {
	remove_btn->connect(SceneStringName(pressed), p_callable, CONNECT_DEFERRED);
}

void ProjectTemplateItem::connect_delete_button(const Callable p_callable) {
	delete_btn->connect(SceneStringName(pressed), p_callable, CONNECT_DEFERRED);
}

void ProjectTemplateItem::update_title(const String &p_title, bool p_revert) {
	if (!p_revert && p_title == title) {
		return;
	}
	old_title = p_revert ? old_title : title;
	title = p_title;
	title_label->set_text(TTRC(title));
}

void ProjectTemplateItem::update_description(const String &p_description, bool p_revert) {
	if (!p_revert && p_description == old_description) {
		return;
	}
	old_description = p_revert ? old_description : description;
	description = p_description;
	set_tooltip_text(TTRC(description));
}

void ProjectTemplateItem::save_changes() {
	if (title != old_title) {
		Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
		String new_name = template_dir.get_base_dir().path_join(title);
		d->rename_absolute(template_dir, new_name);

		ProjectTemplate *pt = ProjectTemplate::get_singleton();

		pt->update_templates_quick_access_config(old_title, true);

		if (!pt->templates_quick_access_config_has_key(title)) {
			pt->update_templates_quick_access_config(title, false, true);
		}

		old_title = title;
		template_dir = template_dir.get_base_dir().path_join(title);
	}
	if (description != old_description) {
		ConfigFile config;

		String path = template_dir.path_join("template.cfg");
		config.load(path);
		config.set_value("description", "description", description);
		config.save(path);

		old_description = description;
	}
}

void ProjectTemplateItem::revert_changes() {
	update_title(old_title, true);
	update_description(old_description, true);
}

void ProjectTemplateItem::_open_template_folder() {
	OS::get_singleton()->shell_show_in_file_manager(template_dir, true);
}

void ProjectTemplateItem::_set_template_icon() {
	icon_loaded = true;

	Ref<Texture2D> default_icon = get_editor_theme_icon("ProjectTemplateIcon");
	Ref<Texture2D> t_icon;

	Ref<Image> img;
	img.instantiate();
	Error err = img->load(template_dir.path_join("icon.svg"));
	if (err == OK) {
		img->resize(default_icon->get_width(), default_icon->get_height(), Image::INTERPOLATE_LANCZOS);
		t_icon = ImageTexture::create_from_image(img);
	}

	if (t_icon.is_null()) {
		t_icon = default_icon;
	}
	icon->set_expand_mode(TextureRect::EXPAND_IGNORE_SIZE);
	icon->set_custom_minimum_size(Size2(48, 48) * EDSCALE);
	icon->set_stretch_mode(TextureRect::STRETCH_KEEP_ASPECT_CENTERED);

	icon->set_texture(t_icon);
}

// Object methods.

ProjectTemplateItem::ProjectTemplateItem(const String &p_title, const String &p_description) {
	set_focus_mode(FocusMode::FOCUS_ALL);

	title = p_title;
	old_title = title;
	description = p_description;
	old_description = description;

	set_tooltip_text(TTRC(description));

	set_h_size_flags(Control::SIZE_EXPAND_FILL);
	set_v_size_flags(Control::SIZE_SHRINK_CENTER);

	// Left spacer.

	add_child(memnew(Control));

	// Left half.

	HBoxContainer *half = memnew(HBoxContainer);
	half->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	add_child(half);

	VBoxContainer *vb = memnew(VBoxContainer);
	vb->set_alignment(ALIGNMENT_CENTER);
	half->add_child(vb);

	icon = memnew(TextureRect);
	vb->add_child(icon);

	vb = memnew(VBoxContainer);
	vb->set_alignment(ALIGNMENT_CENTER);
	half->add_child(vb);

	title_label = memnew(Label(TTRC(title)));
	vb->add_child(title_label);

	// Right half.

	half = memnew(HBoxContainer);
	half->set_alignment(ALIGNMENT_END);
	half->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	add_child(half);

	open_explorer_btn = memnew(Button);
	open_explorer_btn->set_name(TTRC("Open in File Explorer"));
	open_explorer_btn->set_accessibility_name(vformat(TTR("Open Project Template: %s in File Explorer"), p_title));
	open_explorer_btn->set_tooltip_text(TTRC("Open in File Explorer"));
	open_explorer_btn->set_icon_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	open_explorer_btn->set_v_size_flags(Control::SIZE_SHRINK_CENTER);
	open_explorer_btn->set_mouse_filter(MOUSE_FILTER_PASS);
	open_explorer_btn->connect(SceneStringName(pressed), callable_mp(this, &ProjectTemplateItem::_open_template_folder));
	half->add_child(open_explorer_btn);

	edit_btn = memnew(Button);
	edit_btn->set_name(TTRC("Edit Template"));
	edit_btn->set_accessibility_name(vformat(TTR("Edit Project Template: %s"), p_title));
	edit_btn->set_tooltip_text(TTRC("Edit Template Name & Description"));
	edit_btn->set_icon_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	edit_btn->set_v_size_flags(Control::SIZE_SHRINK_CENTER);
	edit_btn->set_mouse_filter(MOUSE_FILTER_PASS);
	half->add_child(edit_btn);

	remove_btn = memnew(Button);
	remove_btn->set_name(TTRC("Remove Template"));
	remove_btn->set_accessibility_name(vformat(TTR("Remove Project Template: %s"), p_title));
	remove_btn->set_tooltip_text(TTRC("Remove Template from List"));
	remove_btn->set_icon_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	remove_btn->set_v_size_flags(Control::SIZE_SHRINK_CENTER);
	remove_btn->set_mouse_filter(MOUSE_FILTER_PASS);
	half->add_child(remove_btn);

	half->add_child(memnew(VSeparator));

	delete_btn = memnew(Button);
	delete_btn->set_name(TTRC("Remove Template"));
	delete_btn->set_accessibility_name(vformat(TTR("Delete Project Template: %s"), p_title));
	delete_btn->set_tooltip_text(TTRC("Delete Template"));
	delete_btn->set_icon_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	delete_btn->set_v_size_flags(Control::SIZE_SHRINK_CENTER);
	delete_btn->set_mouse_filter(MOUSE_FILTER_PASS);
	half->add_child(delete_btn);

	// Right spacer.

	add_child(memnew(Control));
}

ProjectTemplate *ProjectTemplate::singleton = nullptr;

void ProjectTemplate::initialize(bool p_is_editor) {
	ProjectTemplate *pt = memnew(ProjectTemplate(p_is_editor));
	SceneTree::get_singleton()->get_root()->add_child(pt);
}

void ProjectTemplate::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_THEME_CHANGED: {
			if (!is_editor) {
				add_btn->set_icon_alignment(HORIZONTAL_ALIGNMENT_LEFT);
				add_btn->set_button_icon(get_editor_theme_icon(SNAME("FolderBrowse")));
			}

			browse_btn->set_icon_alignment(HORIZONTAL_ALIGNMENT_LEFT);
			browse_btn->set_button_icon(get_editor_theme_icon(SNAME("FolderBrowse")));
		} break;

		case NOTIFICATION_PROCESS: {
			if (scan_data && scan_data->scan_in_progress.is_set()) {
				// Wait for the thread.
			} else {
				set_process(false);
				if (scan_data) {
					_scan_finished();
				}
			}
		} break;
	}
}

// Directory scan.

void ProjectTemplate::_scan_thread(void *p_scan_data) {
	ScanData *scan_data = static_cast<ScanData *>(p_scan_data);

	for (const String &base_path : scan_data->paths_to_scan) {
		print_verbose(vformat("Scanning for files and directories in \"%s\".", base_path));
		_scan_dir_recursive(base_path, &scan_data->dir_items, scan_data->scan_in_progress);

		if (!scan_data->scan_in_progress.is_set()) {
			print_verbose("Scan aborted.");
			break;
		}
	}
	print_verbose(vformat("Found %d files and directories.", scan_data->dir_items.size()));
	scan_data->scan_in_progress.clear();
}

void ProjectTemplate::_scan_finished() {
	if (scan_data->scan_in_progress.is_set()) {
		// Abort scanning.
		scan_data->scan_in_progress.clear();
	}

	scan_data->thread->wait_to_finish();
	memdelete(scan_data->thread);
	if (scan_progress) {
		scan_progress->hide();
	}

	PackedStringArray files;
	for (const String &dir : scan_data->dir_items) {
		if (DirAccess::dir_exists_absolute(dir)) {
			folders.append(dir);
		} else if (FileAccess::exists(dir)) {
			files.append(dir);
		}
	}
	memdelete(scan_data);
	scan_data = nullptr;

	for (const String &file : files) {
		if (script_ext.has(file.get_extension())) {
			script_files.append(file);
			script_ext[file.get_extension()] = true;
		} else if (file.ends_with(".tscn")) {
			scene_files.append(file);
		} else if (file.ends_with(".tres")) {
			tres_files.append(file);
		} else if (file.ends_with(".uid")) {
			script_files.append(file);
		}
	}
	bool scr = false;
	bool scn = false;
	bool tres = false;
	if (!folders.is_empty()) {
		folders_cb->set_pressed(true);
		folders_cb->set_disabled(false);
	}
	if (!script_files.is_empty()) {
		file_type["script"] = true;
		scr = true;
	}
	if (!scene_files.is_empty()) {
		file_type["scene"] = true;
		scn = true;
	}
	if (!tres_files.is_empty()) {
		file_type["tres"] = true;
		tres = true;
	}
	if (scr || scn || tres) {
		files_cb->set_disabled(false);
		files_cb->set_pressed(true);
		files_vb->show();
	}
	scanned_dir = project_dir;

	_update_tree_items();
	_open_create_dialog(false);
}

void ProjectTemplate::_scan_dir_recursive(const String &p_path, List<String> *r_dirs, const SafeFlag &p_scan_active) {
	if (!p_scan_active.is_set()) {
		return;
	}

	Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	Error error = d->change_dir(p_path);
	ERR_FAIL_COND_MSG(error != OK, vformat("Failed to open the path \"%s\" for scanning (code %d).", p_path, error));

	d->list_dir_begin();
	String n = d->get_next();
	while (!n.is_empty()) {
		if (!p_scan_active.is_set()) {
			return;
		}
		if (d->current_is_dir() && !n.begins_with(".")) {
			r_dirs->push_back(d->get_current_dir().path_join(n));
			_scan_dir_recursive(d->get_current_dir().path_join(n), r_dirs, p_scan_active);
		} else if (!n.begins_with(".")) {
			r_dirs->push_back(d->get_current_dir().path_join(n));
		}
		n = d->get_next();
	}
	d->list_dir_end();
}

void ProjectTemplate::_scan_project_dir() {
	if (!scan_progress && is_inside_tree()) {
		scan_progress = memnew(AcceptDialog);
		scan_progress->set_title(TTRC("Scanning"));
		scan_progress->set_ok_button_text(TTRC("Cancel"));

		VBoxContainer *vb = memnew(VBoxContainer);
		scan_progress->add_child(vb);

		Label *label = memnew(Label);
		label->set_text(TTRC("Scanning for projects..."));
		vb->add_child(label);

		ProgressBar *progress = memnew(ProgressBar);
		progress->set_indeterminate(true);
		vb->add_child(progress);

		add_child(scan_progress);
		scan_progress->connect(SceneStringName(confirmed), callable_mp(this, &ProjectTemplate::_scan_finished));
		scan_progress->connect("canceled", callable_mp(this, &ProjectTemplate::_scan_finished));
	}

	scan_data = memnew(ScanData);
	scan_data->paths_to_scan = PackedStringArray({ project_dir });
	scan_data->scan_in_progress.set();

	scan_data->thread = memnew(Thread);
	scan_data->thread->start(_scan_thread, scan_data);

	if (scan_progress) {
		scan_progress->reset_size();
		scan_progress->popup_centered();
	}
	set_process(true);
}

void ProjectTemplate::_cleanup_previous_scan() {
	folders.clear();
	script_files.clear();
	scene_files.clear();
	tres_files.clear();

	settings_cb->set_disabled(true);
	folders_cb->set_disabled(true);
	files_cb->set_disabled(true);
	files_vb->hide();

	all_include->set_checked(0, false);
	all_include->propagate_check(0);

	for (KeyValue<String, bool> &type : file_type) {
		if (type.value) {
			type.value = false;
		}
	}
	for (KeyValue<String, bool> &ext : script_ext) {
		if (ext.value) {
			ext.value = false;
		}
	}
	for (KeyValue<TreeItem *, bool> &item : file_tree_items) {
		if (item.value) { // remove TreeItem if it is in the tree.
			_update_tree_item(item.key, script_include, false);
		}
	}
}

// Directory operations.

void ProjectTemplate::set_project_path(const String &p_path) {
	project_dir = p_path;
}

void ProjectTemplate::_set_templates_dir(const String p_text) {
	EditorSettings::get_singleton()->set_setting(templates_dir_setting, p_text);
	templates_dir = p_text;
}

void ProjectTemplate::_dir_selected(const String &p_dir) {
	if (mode == MODE_MANAGE) {
		_update_template_list(p_dir);
		_manage_templates();
	} else if (mode == MODE_CREATE) {
		path_edit->set_text(p_dir);
		path_edit->emit_signal(SceneStringName(text_changed), p_dir);
		create_dialog->popup_centered(Size2(500 * EDSCALE, 0));
	}
}

void ProjectTemplate::_create_templates_dir() {
	String path = path_edit->get_text().simplify_path();

	Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	if (!d->dir_exists(path) && d->make_dir(path) != OK) {
		_set_message(TTRC("Couldn't create templates directory, check permissions."), MESSAGE_ERROR, PATH);
		return;
	}

	_set_templates_dir(path);
}

Error ProjectTemplate::_create_folders() {
	const String path = templates_dir.path_join(template_folder).path_join("directories.cfg");
	ConfigFile config;
	Error err = config.load(path);
	if (err != OK) {
		_show_error(vformat(TTR("Cannot load directories.cfg file in '%s', it may be missing or corrupted\nTemplate copying aborted."), path.get_base_dir()));
		return err;
	}
	const PackedStringArray dirs = config.get_section_keys("directories");

	Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	for (const String &dir : dirs) {
		const String new_dir = project_dir.path_join(dir);
		if (!d->dir_exists(new_dir)) {
			err = d->make_dir(new_dir);
			if (err != OK) {
				_show_error(TTRC("Couldn't create template folder, check permissions."));
				return err;
			}
		}
	}
	return OK;
}

PackedStringArray ProjectTemplate::_copy_files_all() {
	PackedStringArray error_string;

	const String path = templates_dir.path_join(template_folder).path_join("file_locations.cfg");
	ConfigFile config;
	Error err = config.load(path);
	if (err != OK) {
		error_string.append(vformat(TTR("Cannot load file_locations.cfg file in '%s', it may be missing or corrupted\nTemplate copying aborted."), path.get_base_dir()));
		return error_string;
	}
	const String from = templates_dir.path_join(template_folder).path_join("Files");
	const PackedStringArray sections = config.get_sections();

	Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	for (const String &section : sections) {
		PackedStringArray dirs = config.get_section_keys(section);
		for (const String &dir : dirs) {
			const String file = dir.get_slice("/", dir.get_slice_count("/") - 1);
			const String new_dir = project_dir.path_join(dir);

			err = d->copy_absolute(from.path_join(file), new_dir);
			if (err != OK) {
				error_string.append(vformat(TTR("Failed to copy file %s from\n %s, (error %d)"), from, new_dir, err));
			}
		}
	}

	return error_string;
}

PackedStringArray ProjectTemplate::_copy_files_individual(const PackedStringArray &p_from, const String &p_to, const String &p_filter) {
	PackedStringArray error_string;

	Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	for (const String &file : p_from) {
		if (!folders_cb->is_pressed() && file.get_base_dir() != project_dir) {
			continue;
		}
		const String file_name = file.get_slice("/", file.get_slice_count("/") - 1);
		if (p_filter != file_name.get_extension()) {
			continue;
		}

		Error err = d->copy_absolute(file, p_to.path_join(file_name));
		if (err != OK) {
			error_string.append(vformat(TTR("Failed to copy file %s from\n %s, (error %d)"), file_name, file, err));
		} else {
			const PackedStringArray split = file.split("/", false, project_dir.get_slice_count("/"));
			const String strip_file = split[split.size() - 1];
			const String section = p_filter == "tscn" ? "scenes" : "resources";
			file_config.set_value(section, strip_file, false);
		}
	}

	return error_string;
}

PackedStringArray ProjectTemplate::_copy_script_files_individual(const PackedStringArray &p_from, const String &p_to, const String &p_filter) {
	PackedStringArray error_string;

	Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	for (const String &file : p_from) {
		if (!folders_cb->is_pressed() && file.get_base_dir() != project_dir) {
			continue;
		}

		const String file_name = file.get_slice("/", file.get_slice_count("/") - 1);
		if (p_filter != file_name.get_extension()) {
			continue;
		}

		Error err = d->copy_absolute(file, p_to.path_join(file_name));
		if (err != OK) {
			error_string.append(vformat(TTR("Failed to copy file %s from\n %s, (error %d)"), file_name, file, err));
		} else {
			const PackedStringArray split = file.split("/", false, project_dir.get_slice_count("/"));
			const String strip_file = split[split.size() - 1];
			file_config.set_value("scripts", strip_file, false);
		}

		// Copy uid files for safe autoload transfer.

		const String f_uid = file + ".uid";
		const String fn_uid = file_name + ".uid";
		err = d->copy_absolute(f_uid, p_to.path_join(fn_uid));
		if (err != OK) {
			error_string.append(vformat(TTR("Failed to copy file %s from\n %s, (error %d)"), fn_uid, f_uid, err));
		} else {
			const PackedStringArray split = f_uid.split("/", false, project_dir.get_slice_count("/"));
			const String strip_f_uid = split[split.size() - 1];
			file_config.set_value("scripts", strip_f_uid, false);
		}
	}

	return error_string;
}

// Template creation management.

void ProjectTemplate::_set_name(const String p_text, bool p_edit) {
	String full_path;
	if (!templates_dir.is_empty()) {
		full_path = templates_dir.path_join(p_text).simplify_path().strip_edges();
	} else {
		full_path = OS::get_singleton()->has_environment("HOME") ? OS::get_singleton()->get_environment("HOME") : OS::get_singleton()->get_system_dir(OS::SYSTEM_DIR_DOCUMENTS);
	}
	Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);

	ProjectTemplateItem *edit = nullptr;
	if (p_edit) {
		edit = edited_templates[edited_templates.size() - 1];
	}

	if (edit && edit->title != p_text && template_names.has(p_text)) {
		_set_message(TTRC("Template with name already exists"), MESSAGE_ERROR, NAME, p_edit);
		goto update_ok_button;
	} else if (!p_edit && template_names.has(p_text)) {
		_set_message(TTRC("Template with name already exists, data and files will be overwritten."), MESSAGE_WARNING, NAME, p_edit);
		goto update_ok_button;
	}

	if (p_text.is_empty()) {
		_set_message(TTRC("It would be a good idea to name your template"), MESSAGE_ERROR, NAME, p_edit);
		goto update_ok_button;
	}

	if (full_path.get_file() != OS::get_singleton()->get_safe_dir_name(full_path.get_file())) {
		_set_message(TTRC("The template name must also be a valid folder name."), MESSAGE_ERROR, NAME, p_edit);
		goto update_ok_button;
	} else {
		_set_message(TTRC("Template name valid"), MESSAGE_SUCCESS, NAME, p_edit);
	}

	if (templates_dir.is_empty() && path_edit->get_text().is_empty()) {
		_set_message(TTRC("Select a directory to save template"), MESSAGE_ERROR, PATH);
		goto update_ok_button;
	}

update_ok_button:
	_update_dialog_ok_button();
}

void ProjectTemplate::_set_path(const String p_path) {
	if (!path_container->is_visible()) {
		return;
	}

	String path = p_path.simplify_path();
	Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	if (path.is_relative_path()) {
		_set_message(TTRC("The path specified is invalid."), MESSAGE_ERROR, PATH);
		goto update_ok_button;
	}

	if (!d->dir_exists(path.get_base_dir())) {
		_set_message(TTRC("The parent directory of the path specified doesn't exist."), MESSAGE_ERROR, PATH);
		goto update_ok_button;
	}

	if (d->dir_exists(path)) {
		bool is_folder_empty = true;
		if (d->change_dir(path) == OK) {
			d->list_dir_begin();
			String n = d->get_next();
			while (!n.is_empty()) {
				if (n[0] != '.') {
					is_folder_empty = false;
					break;
				}
				n = d->get_next();
			}
			d->list_dir_end();

			if (!is_folder_empty) {
				_set_message(TTRC("Cannot make non-empty directory templates directory"), MESSAGE_ERROR, PATH);
				goto update_ok_button;

			} else {
				_set_message(TTRC("Will set templates folder"), MESSAGE_SUCCESS, PATH);
			}
		}
	}

update_ok_button:
	_update_dialog_ok_button();
}

void ProjectTemplate::_set_message(const String &p_msg, MessageType p_type, InputType p_input_type, bool p_edit) {
	Ref<Texture2D> status_icon;
	Color err_color;
	switch (p_type) {
		case MESSAGE_ERROR: {
			err_color = get_theme_color(SNAME("error_color"), EditorStringName(Editor));
			status_icon = get_editor_theme_icon(SNAME("StatusError"));
			status_red = true;
		} break;
		case MESSAGE_WARNING: {
			err_color = get_theme_color(SNAME("warning_color"), EditorStringName(Editor));
			status_icon = get_editor_theme_icon(SNAME("StatusWarning"));
			status_red = false;
		} break;
		case MESSAGE_SUCCESS: {
			err_color = get_theme_color(SNAME("success_color"), EditorStringName(Editor));
			status_icon = get_editor_theme_icon(SNAME("StatusSuccess"));
			status_red = false;
		} break;
	}

	if (p_input_type == NAME) {
		if (p_edit) {
			title_status_label->set_text(p_msg);
			title_status_label->add_theme_color_override(SceneStringName(font_color), err_color);
			title_status_rect->set_texture(status_icon);
		} else {
			name_status_label->set_text(p_msg);
			name_status_label->add_theme_color_override(SceneStringName(font_color), err_color);
			name_status_rect->set_texture(status_icon);
		}
	} else if (p_input_type == PATH) {
		path_status_label->set_text(p_msg);
		path_status_label->add_theme_color_override(SceneStringName(font_color), err_color);
		path_status_rect->set_texture(status_icon);
	}
}

void ProjectTemplate::_update_dialog_ok_button() {
	if (mode == MODE_CREATE) {
		create_dialog->get_ok_button()->set_disabled(status_red);
	} else {
		edit_dialog->get_ok_button()->set_disabled(status_red);
	}
}

// Manage project templates.

void ProjectTemplate::_load_template_list(bool p_load_template_items) {
	if (templates_dir.is_empty()) {
		return;
	}

	ConfigFile config;
	Error err = config.load(templates_dir.path_join(".templates"));
	if (err != OK) {
		return;
	}

	PackedStringArray t_names = config.get_section_keys("templates");

	for (const String &t : t_names) {
		template_names[t] = false;

		if (p_load_template_items) {
			bool add_to_list = config.get_value("templates", t, false);
			if (add_to_list) {
				const String path = templates_dir.path_join(t).path_join("template.cfg");
				ConfigFile c;
				err = c.load(path);
				if (err != OK) {
					config.erase_section_key("templates", t);
					continue;
				}
				const String title = t;
				const String description = c.get_value("description", "description", String());
				ProjectTemplateItem *item = memnew(ProjectTemplateItem(title, description));
				item->set_template(path.get_base_dir());
				item->connect_edit_button(callable_mp(this, &ProjectTemplate::_edit_template).bind(item));
				item->connect_remove_button(callable_mp(this, &ProjectTemplate::_remove_template).bind(item));
				item->connect_delete_button(callable_mp(this, &ProjectTemplate::_show_delete_template_warning_dialog).bind(item));
				template_items.push_back(item);
				template_names[title] = true;
			}
		}
	}
	template_items.sort_custom<ProjectTemplateComparator>();
}

void ProjectTemplate::_update_template_list(const String &p_template) {
	const String title = p_template.get_slice("/", p_template.get_slice_count("/") - 1);
	if (template_names.has(title) && template_names[title] == true) {
		return;
	}

	ConfigFile config;
	Error err = config.load(p_template.path_join("template.cfg"));
	if (err != OK) {
		_show_error(vformat(TTR("cannot load template.cfg file from directory '%s'.\nThe file may be missing or corrupted."), p_template));
		return;
	}

	const String description = config.get_value("description", "description", String());
	ProjectTemplateItem *item = memnew(ProjectTemplateItem(title, description));
	item->set_template(p_template);
	item->connect_edit_button(callable_mp(this, &ProjectTemplate::_edit_template).bind(item));
	item->connect_remove_button(callable_mp(this, &ProjectTemplate::_remove_template).bind(item));
	item->connect_delete_button(callable_mp(this, &ProjectTemplate::_show_delete_template_warning_dialog).bind(item));
	template_items.push_back(item);
	template_items.sort_custom<ProjectTemplateComparator>();

	template_names[title] = true;
	update_templates_quick_access_config(title, false, true);
}

void ProjectTemplate::_manage_templates() {
	if (template_items.size() == 0) {
		no_templates_label->set_visible(true);
		templates_container->set_visible(false);
	} else if (templates_container->get_child_count() != template_items.size()) {
		no_templates_label->set_visible(false);
		templates_container->set_visible(true);

		for (Node *child : templates_container->iterate_children()) {
			ProjectTemplateItem *item = cast_to<ProjectTemplateItem>(child);
			if (item) {
				templates_container->remove_child(item);
			}
		}
		for (ProjectTemplateItem *item : template_items) {
			templates_container->add_child(item);
		}
	}

	manage_dialog->popup_centered(Size2(300, 400) * EDSCALE);
}

void ProjectTemplate::_apply_template_changes(bool p_canceled) {
	if (p_canceled) {
		if (edited_templates.size() > 0) {
			for (ProjectTemplateItem *item : edited_templates) {
				item->revert_changes();
			}
			for (PackedStringArray &t_names : template_name_archive) {
				template_names.erase(t_names[0]);
				template_names[t_names[1]] = true;
			}
			template_name_archive.clear();
		}
		for (ProjectTemplateItem *item : remove_templates) {
			template_names[item->title] = true;
		}
		edited_templates.clear();
		remove_templates.clear();
		delete_templates.clear();

		return;
	}

	if (edited_templates.size() > 0) {
		for (ProjectTemplateItem *item : edited_templates) {
			item->save_changes();
		}
		edited_templates.clear();
		template_name_archive.clear();
	}
	if (delete_templates.size() > 0) {
		for (ProjectTemplateItem *item : delete_templates) {
			update_templates_quick_access_config(item->title, true);
			_delete_template_folder(item);
			template_names.erase(item->title);
			template_items.erase(item);
			memdelete(item);
		}
		delete_templates.clear();
	}
	if (remove_templates.size() > 0) {
		for (ProjectTemplateItem *item : remove_templates) {
			update_templates_quick_access_config(item->title, false);
			template_items.erase(item);
			memdelete(item);
		}
		remove_templates.clear();
	}
}

void ProjectTemplate::select_template_item(int p_index, bool p_hide_focus) {
	for (ProjectTemplateItem *item : template_items) {
		int idx = item->get_index();
		if (idx == p_index) {
			item->set_selected(true, p_hide_focus);
			continue;
		}
		item->set_selected(false);
	}
}

void ProjectTemplate::_remove_template(ProjectTemplateItem *p_item) {
	templates_container->remove_child(p_item);
	remove_templates.append(p_item);
	template_names[p_item->title] = false;
}

void ProjectTemplate::_delete_template(ProjectTemplateItem *p_item) {
	templates_container->remove_child(p_item);
	delete_templates.append(p_item);
	template_names[p_item->title] = false;
}

void ProjectTemplate::_delete_template_folder(ProjectTemplateItem *p_item) {
	Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	const String dir = p_item->get_template();
	if (!d->dir_exists(dir)) {
		return;
	}
	d->change_dir(dir);
	d->erase_contents_recursive();
	d->remove_absolute(dir);
}

void ProjectTemplate::_show_delete_template_warning_dialog(ProjectTemplateItem *p_item) {
	bool bypass = Input::get_singleton()->is_physical_key_pressed(Key::SHIFT);
	if (bypass) {
		_delete_template(p_item);
	} else {
		delete_dialog->connect(SceneStringName(confirmed), callable_mp(this, &ProjectTemplate::_delete_template).bind(p_item), CONNECT_ONE_SHOT);
		delete_dialog->popup_centered(Size2(400 * EDSCALE, 0));
	}
}

// Edit template.

void ProjectTemplate::_edit_template(ProjectTemplateItem *p_item) {
	if (edited_templates.has(p_item)) {
		edited_templates.erase(p_item);
	}
	edited_templates.push_back(p_item);

	title_edit->set_text(p_item->title);
	desc_edit->set_text(p_item->description);

	_set_name(p_item->title, true);
	edit_dialog->popup_centered(Size2(300 * EDSCALE, 0));
	title_edit->grab_focus();
	title_edit->select_all();
}

void ProjectTemplate::_edit_template_confirmed() {
	ProjectTemplateItem *item = edited_templates[edited_templates.size() - 1];

	const String title = title_edit->get_text();
	const String desc = desc_edit->get_text();
	const String old_title = item->title;

	item->update_title(title);
	item->update_description(desc);

	PackedStringArray t_names = { title, old_title };
	template_name_archive.append(t_names);
	template_names.erase(old_title);
	template_names[title] = true;
}

// Create template.

bool ProjectTemplate::_get_changed_settings() {
	String project = project_dir.path_join("project.godot");

	ProjectSettings *cfg = memnew(ProjectSettings(project));
	if (!cfg->is_project_loaded()) {
		memdelete(cfg);
		_show_error(vformat(TTR("Couldn't read from project.godot file at '%s'. It may be missing or corrupted."), project), Vector2(300 * EDSCALE, 0));
		return false;
	}

	template_config.set_value("description", "description", String());

	const PackedStringArray settings = cfg->get_changed_settings();
	const ProjectSettings::CustomMap initial_settings = EditorNode::get_initial_settings();
	for (const String &setting : settings) {
		if (setting.begins_with("application")) {
			continue;
		}
		Variant value = cfg->get_setting(setting);
		if (initial_settings.has(setting) && initial_settings[setting] == value) {
			continue;
		}
		const PackedStringArray slices = setting.split("/", false, 1);
		const String section = slices[0];
		const String key = slices[1];
		template_config.set_value(section, key, value);
	}

	if (template_config.get_sections().size() > 0) {
		settings_cb->set_disabled(false);
		settings_cb->set_pressed(true);
	}

	return true;
}

void ProjectTemplate::_open_create_dialog(bool p_reset_name) {
	templates_dir = EDITOR_GET(templates_dir_setting);
	path_container->set_visible(templates_dir.is_empty());
	const Size2 minsize = path_container->is_visible() ? Size2(500 * EDSCALE, 0) : Size2(400 * EDSCALE, 0);
	template_desc->clear();

	if (p_reset_name) {
		String t_name;
		switch (naming_convention) {
			case 0: // No Convention
				break;
			case 1: // kebab-case
				t_name = default_name.to_kebab_case();
				break;
			case 2: // snake_case
				t_name = default_name.to_snake_case();
				break;
			case 3: // camelCase
				t_name = default_name.to_camel_case();
				break;
			case 4: // PascalCase
				t_name = default_name.to_pascal_case();
				break;
			case 5: // Title Case
				t_name = default_name.capitalize();
				break;
			default:
				ERR_FAIL_MSG("Invalid directory naming convention.");
				break;
		}
		name_edit->set_text(TTRC(t_name));
		_set_name(t_name, false);
	}

	if (scanned_dir != project_dir) {
		_cleanup_previous_scan();
		bool success = _get_changed_settings();
		if (success) {
			_scan_project_dir();
		} else {
			return;
		}
	} else {
		create_dialog->popup_centered(minsize);
		name_edit->grab_focus();
		name_edit->select_all();
	}
}

void ProjectTemplate::_create_template() {
	template_folder = templates_dir.path_join(name_edit->get_text());

	Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	if (d->dir_exists(template_folder)) {
		d->change_dir(template_folder);
		d->erase_contents_recursive();
		d->remove_absolute(template_folder);
	}
	if (!d->dir_exists(template_folder) && d->make_dir(template_folder) != OK) {
		_show_error(TTRC("Couldn't create template folder, check permissions."));
		return;
	}

	_save_template_config();
	_save_folder_config();
	_copy_files_to_template_dir();

	_update_template_list(template_folder);

	if (!error_dialog->is_visible()) {
		success_dialog->popup_centered();
	}
}

void ProjectTemplate::_copy_files_to_template_dir() {
	if (!all_include->is_checked(0) && !all_include->is_indeterminate(0)) {
		return;
	}

	const String file_dir = template_folder.path_join("Files");
	Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	if (!d->dir_exists(file_dir) && d->make_dir(file_dir) != OK) {
		_show_error(TTRC("Couldn't create file folder within template directory."));
		return;
	}

	PackedStringArray error_string;
	if (scene_include->is_checked(0)) {
		PackedStringArray err = _copy_files_individual(scene_files, file_dir, "tscn");
		error_string.append_array(err);
	}
	if (tres_include->is_checked(0)) {
		PackedStringArray err = _copy_files_individual(tres_files, file_dir, "tres");
		error_string.append_array(err);
	}
	if (script_include->is_checked(0)) {
		for (KeyValue<String, bool> &ext : script_ext) {
			PackedStringArray err = _copy_script_files_individual(script_files, file_dir, ext.key);
			error_string.append_array(err);
		}
	}

	if (!error_string.is_empty()) {
		String error;
		for (const String &err : error_string) {
			error += err + "\n";
		}
		_show_error(error, Size2(500 * EDSCALE, 0));
	}
	_save_file_config();
}

void ProjectTemplate::_cleanup_template_files_and_dirs() {
	String prev_dir;

	Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	d->set_include_hidden(false);
	d->set_include_navigational(false);
	d->change_dir(project_dir);
	PackedStringArray dirs = d->get_directories();
	for (const String &dir : dirs) {
		if (!prev_dir.is_empty()) {
			d->remove_absolute(prev_dir);
		}
		d->change_dir(project_dir.path_join(dir));
		d->erase_contents_recursive();
		prev_dir = d->get_current_dir();
	}
}

// Project creation.

void ProjectTemplate::template_selected(const String &p_project_path, const String &p_template) {
	if (p_template == "None") {
		return;
	}
	project_dir = p_project_path;
	template_folder = p_template;
	template_valid = true;
}

void ProjectTemplate::copy_to_project_dir() {
	PackedStringArray error_strings;

	Error err = _copy_template_settings();
	if (err != OK) {
		return;
	}

	err = _create_folders();
	if (err == OK) {
		error_strings = _copy_files_all();
	}

	if (!error_strings.is_empty()) {
		String error;
		for (const String &error_string : error_strings) {
			error += error_string + "\n";
		}
		_show_error(error, Size2(500 * EDSCALE, 0));
	}
}

Error ProjectTemplate::_copy_template_settings() {
	String project = project_dir.path_join("project.godot");

	ProjectSettings *cfg = memnew(ProjectSettings(project));
	if (!cfg->is_project_loaded()) {
		memdelete(cfg);
		_show_error(vformat(TTR("Couldn't read from project.godot file at '%s'. It may be missing or corrupted. Template porting aborted."), project), Vector2(300 * EDSCALE, 0));
		return ERR_FILE_CANT_OPEN;
	}

	String path = templates_dir.path_join(template_folder).path_join("template.cfg");
	ConfigFile config;
	Error err = config.load(path);
	if (err != OK) {
		_show_error(vformat(TTR("Couldn't load template.cfg file at '%s'. It may be missing or corrupted. Template porting aborted."), path.get_base_dir()), Vector2(300 * EDSCALE, 0));
		return err;
	}

	const PackedStringArray sections = config.get_sections();
	for (const String &section : sections) {
		if (section == "description") {
			continue;
		}
		const PackedStringArray keys = config.get_section_keys(section);
		for (const String &key : keys) {
			const String setting = section.path_join(key);
			const Variant value = config.get_value(section, key, Variant());
			cfg->set(setting, value);
		}
	}

	err = cfg->save_custom(project);
	if (err != OK) {
		_show_error(vformat(TTR("Couldn't save project at '%s' (error %d). Template porting aborted."), project, err));
		return err;
	}

	return OK;
}

// Template dialog.

void ProjectTemplate::set_mode(Mode p_mode) {
	mode = p_mode;
}

void ProjectTemplate::show_dialog(bool p_reset_name) {
	switch (mode) {
		case MODE_MANAGE: {
			_manage_templates();
		} break;

		case MODE_CREATE: {
			_open_create_dialog(p_reset_name);
		} break;
	}
}

// Other dialogs.

void ProjectTemplate::_show_file_dialog() {
	if (mode == MODE_MANAGE) {
		manage_dialog->hide();
		fdialog->set_file_mode(FileDialog::FILE_MODE_OPEN_DIR);
		fdialog->set_current_dir(templates_dir);
		fdialog->popup_file_dialog();
	} else if (mode == MODE_CREATE) {
		create_dialog->hide();
		fdialog->set_file_mode(FileDialog::FILE_MODE_OPEN_FILE);
		fdialog->add_filter("template.cfg");
		fdialog->popup_file_dialog();
	}
}

void ProjectTemplate::_show_error(const String &p_error, Size2i p_minsize) {
	error_label->set_text(p_error);
	error_label->set_custom_minimum_size(p_minsize);
	error_dialog->popup_centered(p_minsize);
}

// Template includes.

void ProjectTemplate::_create_tree() {
	file_tree->set_v_scroll_enabled(false);

	all_include = file_tree->create_item();
	all_include->set_cell_mode(0, TreeItem::CELL_MODE_CHECK);
	all_include->set_editable(0, true);
	all_include->set_text(0, TTRC("Include all files"));

	script_include = create_tree_item(TTRC("Include script files"), all_include);
	scene_include = create_tree_item(TTRC("Include scene (.tscn) files"), all_include);
	tres_include = create_tree_item(TTRC("Include Resource (.tres) files"), all_include);

	for (KeyValue<TreeItem *, bool> &item : file_tree_items) {
		item.value = false;
		_update_tree_item(item.key, all_include, false);
	}
}

TreeItem *ProjectTemplate::create_tree_item(const String &p_item_text, TreeItem *p_parent) {
	TreeItem *item = p_parent->create_child();
	item->set_cell_mode(0, TreeItem::CELL_MODE_CHECK);
	item->set_editable(0, true);
	item->set_text(0, p_item_text);

	return item;
}

void ProjectTemplate::_update_tree_items() {
	for (KeyValue<String, bool> &type : file_type) {
		if (type.value == true) {
			if (type.key == "script") {
				_update_tree_item(script_include, all_include, true);
			} else if (type.key == "scene") {
				_update_tree_item(scene_include, all_include, true);
			} else if (type.key == "tres") {
				_update_tree_item(tres_include, all_include, true);
			}
		}
	}
}

void ProjectTemplate::_update_tree_item(TreeItem *p_item, TreeItem *p_parent, bool p_add) {
	if (p_add) {
		p_parent->add_child(p_item);
	} else {
		p_parent->remove_child(p_item);
	}

	file_tree_items[p_item] = p_add;
}

void ProjectTemplate::_tree_item_edited() {
	TreeItem *edited = file_tree->get_edited();
	ERR_FAIL_NULL(edited);

	edited->propagate_check(0, false);
}

void ProjectTemplate::_update_possible_includes() {
	if (!folders_cb->is_pressed()) {
		files_cb->set_text(TTRC("Include Files (Only Top Level)"));
	} else {
		files_cb->set_text(TTRC("Include Files"));
	}
}

void ProjectTemplate::_update_file_includes() {
	if (!files_cb->is_pressed()) {
		all_include->set_checked(0, false);
		all_include->propagate_check(0);
		all_include->set_editable(0, false);
		for (KeyValue<TreeItem *, bool> &item : file_tree_items) {
			item.key->set_editable(0, false);
		}
	} else {
		all_include->set_editable(0, true);
		for (KeyValue<TreeItem *, bool> &item : file_tree_items) {
			item.key->set_editable(0, true);
		}
	}
}

// Config.

void ProjectTemplate::_save_folder_config() {
	if (!folders_cb->is_pressed()) {
		return;
	}

	ConfigFile config;
	for (const String &folder : folders) {
		const PackedStringArray split = folder.split("/", false, project_dir.get_slice_count("/"));
		const String strip_dir = split[split.size() - 1];
		if (strip_dir == project_dir) {
			continue;
		}
		config.set_value("directories", strip_dir, false);
	}
	const String path = template_folder.path_join("directories.cfg");
	config.save(path);
}

void ProjectTemplate::_save_file_config() {
	const String path = template_folder.path_join("file_locations.cfg");
	file_config.save(path);
	file_config.clear();
}

void ProjectTemplate::_save_template_config() {
	template_config.set_value("description", "description", template_desc->get_text());

	const String path = template_folder.path_join("template.cfg");
	template_config.save(path);
}

void ProjectTemplate::update_templates_quick_access_config(const String &p_template, bool p_remove_template_key, bool p_load_template) {
	ConfigFile config;
	const String path = templates_dir.path_join(".templates");
	if (config.load(path) != OK) {
		config.save(path);
	}
	if (p_remove_template_key) {
		config.erase_section_key("templates", p_template);
	} else {
		config.set_value("templates", p_template, p_load_template);
	}
	config.save(path);
}

bool ProjectTemplate::templates_quick_access_config_has_key(const String &p_key) {
	const String path = templates_dir.path_join(".templates");
	ConfigFile config;
	Error err = config.load(path);
	if (err == OK) {
		return config.has_section_key("templates", p_key);
	}

	return false;
}

// Object methods.

ProjectTemplate::ProjectTemplate(bool p_is_editor) {
	singleton = this;

	is_editor = p_is_editor;
	mode = is_editor ? MODE_CREATE : MODE_MANAGE;

	// Initialize required editor settings.

	templates_dir = EDITOR_GET(templates_dir_setting);
	naming_convention = (int)EDITOR_GET("project_manager/directory_naming_convention");

	if (!is_editor) {
		// Manage dialog.
		{
			manage_dialog = memnew(ConfirmationDialog);
			manage_dialog->set_title(TTRC("Manage Project Templates"));
			manage_dialog->connect(SceneStringName(confirmed), callable_mp(this, &ProjectTemplate::_apply_template_changes).bind(false));
			manage_dialog->connect("canceled", callable_mp(this, &ProjectTemplate::_apply_template_changes).bind(true));
			add_child(manage_dialog);

			VBoxContainer *vb = memnew(VBoxContainer);
			manage_dialog->add_child(vb);

			Label *label = memnew(Label);
			label->set_text(TTRC("Templates"));
			label->set_theme_type_variation("HeaderMedium");
			label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
			vb->add_child(label);

			no_templates_label = memnew(Label);
			no_templates_label->set_text(TTRC("There are currently no templates, create or add one."));
			vb->add_child(no_templates_label);

			ScrollContainer *sc = memnew(ScrollContainer);
			sc->set_follow_focus(true);
			sc->set_v_size_flags(Control::SIZE_EXPAND_FILL);
			sc->set_horizontal_scroll_mode(ScrollContainer::SCROLL_MODE_DISABLED);
			vb->add_child(sc);

			templates_container = memnew(VBoxContainer);
			templates_container->set_h_size_flags(Control::SIZE_EXPAND_FILL);
			templates_container->set_v_size_flags(Control::SIZE_EXPAND_FILL);
			sc->add_child(templates_container);

			vb->add_child(memnew(HSeparator));

			HBoxContainer *hb = memnew(HBoxContainer);
			hb->set_alignment(BoxContainer::ALIGNMENT_CENTER);
			vb->add_child(hb);

			add_btn = memnew(Button);
			add_btn->set_text(TTRC("Add Template"));
			add_btn->connect(SceneStringName(pressed), callable_mp(this, &ProjectTemplate::_show_file_dialog));
			hb->add_child(add_btn);
		}

		// Edit dialog.
		{
			edit_dialog = memnew(ConfirmationDialog);
			edit_dialog->set_title(TTRC("Edit Template"));
			edit_dialog->connect(SceneStringName(confirmed), callable_mp(this, &ProjectTemplate::_edit_template_confirmed));
			manage_dialog->add_child(edit_dialog);

			VBoxContainer *main_vb = memnew(VBoxContainer);
			edit_dialog->add_child(main_vb);

			VBoxContainer *vb = memnew(VBoxContainer);
			main_vb->add_child(vb);

			Label *label = memnew(Label(TTRC("Title:")));
			vb->add_child(label);

			HBoxContainer *hb = memnew(HBoxContainer);
			vb->add_child(hb);

			title_edit = memnew(LineEdit);
			title_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
			title_edit->connect(SceneStringName(text_changed), callable_mp(this, &ProjectTemplate::_set_name).bind(true));
			hb->add_child(title_edit);

			title_status_rect = memnew(TextureRect);
			title_status_rect->set_stretch_mode(TextureRect::STRETCH_KEEP_CENTERED);
			hb->add_child(title_status_rect);

			title_status_label = memnew(Label);
			vb->add_child(title_status_label);

			vb = memnew(VBoxContainer);
			main_vb->add_child(vb);

			label = memnew(Label(TTRC("Description:")));
			vb->add_child(label);

			desc_edit = memnew(TextEdit);
			desc_edit->set_line_wrapping_mode(TextEdit::LINE_WRAPPING_BOUNDARY);
			desc_edit->set_h_size_flags(Control::SIZE_SHRINK_BEGIN);
			desc_edit->set_custom_minimum_size(Size2(267, 88) * EDSCALE);
			vb->add_child(desc_edit);
		}

		// Delete dialog.

		{
			delete_dialog = memnew(AcceptDialog);
			delete_dialog->set_title(TTRC("Delete Template"));
			delete_dialog->set_text(TTRC("Are you sure you want to delete this template?\nHold shift to bypass this dialog."));
			manage_dialog->add_child(delete_dialog);
		}
	}

	// Create dialog.
	{
		create_dialog = memnew(ConfirmationDialog);
		create_dialog->set_title(TTRC("Create Project Template"));
		create_dialog->connect(SceneStringName(confirmed), callable_mp(this, &ProjectTemplate::_create_template));
		add_child(create_dialog);

		VBoxContainer *main_vb = memnew(VBoxContainer);
		create_dialog->add_child(main_vb);

		VBoxContainer *top_vb = memnew(VBoxContainer);
		main_vb->add_child(top_vb);

		name_container = memnew(VBoxContainer);
		top_vb->add_child(name_container);

		Label *label = memnew(Label(TTRC("Template Name:")));
		name_container->add_child(label);

		HBoxContainer *hb = memnew(HBoxContainer);
		name_container->add_child(hb);

		name_edit = memnew(LineEdit);
		name_edit->set_h_size_flags(Control::SizeFlags::SIZE_EXPAND_FILL);
		name_edit->connect(SceneStringName(text_changed), callable_mp(this, &ProjectTemplate::_set_name).bind(false));
		hb->add_child(name_edit);

		name_status_rect = memnew(TextureRect);
		name_status_rect->set_stretch_mode(TextureRect::STRETCH_KEEP_CENTERED);
		hb->add_child(name_status_rect);

		name_status_label = memnew(Label);
		name_container->add_child(name_status_label);

		path_container = memnew(VBoxContainer);
		top_vb->add_child(path_container);

		label = memnew(Label(TTRC("Templates Folder:")));
		path_container->add_child(label);

		hb = memnew(HBoxContainer);
		path_container->add_child(hb);

		path_edit = memnew(LineEdit);
		path_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		path_edit->connect(SceneStringName(text_changed), callable_mp(this, &ProjectTemplate::_set_path));
		hb->add_child(path_edit);

		path_status_rect = memnew(TextureRect);
		path_status_rect->set_stretch_mode(TextureRect::STRETCH_KEEP_CENTERED);
		hb->add_child(path_status_rect);

		browse_btn = memnew(Button);
		browse_btn->set_text(TTRC("Browse"));
		browse_btn->connect(SceneStringName(pressed), callable_mp(this, &ProjectTemplate::_show_file_dialog));
		hb->add_child(browse_btn);

		path_status_label = memnew(Label);
		path_container->add_child(path_status_label);

		VBoxContainer *vb = memnew(VBoxContainer);
		top_vb->add_child(vb);

		label = memnew(Label(TTRC("Description:")));
		vb->add_child(label);

		template_desc = memnew(TextEdit);
		template_desc->set_line_wrapping_mode(TextEdit::LINE_WRAPPING_BOUNDARY);
		template_desc->set_v_size_flags(Control::SIZE_EXPAND_FILL);
		template_desc->set_custom_minimum_size(Size2(0, 88 * EDSCALE));
		vb->add_child(template_desc);

		VBoxContainer *bottom_vb = memnew(VBoxContainer);
		bottom_vb->set_alignment(BoxContainer::ALIGNMENT_END);
		main_vb->add_child(bottom_vb);

		includes_vb = memnew(VBoxContainer);
		bottom_vb->add_child(includes_vb);

		includes_vb->add_child(memnew(HSeparator));

		label = memnew(Label("Include in Template"));
		label->set_theme_type_variation("HeaderMedium");
		label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
		includes_vb->add_child(label);

		settings_cb = memnew(CheckBox);
		settings_cb->set_text(TTRC("Include Project Settings"));
		includes_vb->add_child(settings_cb);

		folders_cb = memnew(CheckBox);
		folders_cb->set_text(TTRC("Save Folder Structure"));
		folders_cb->connect(SceneStringName(toggled), callable_mp(this, &ProjectTemplate::_update_possible_includes).unbind(1));
		includes_vb->add_child(folders_cb);

		files_cb = memnew(CheckBox);
		files_cb->set_text(TTRC("Include Files"));
		files_cb->connect(SceneStringName(toggled), callable_mp(this, &ProjectTemplate::_update_file_includes).unbind(1));
		includes_vb->add_child(files_cb);

		files_vb = memnew(VBoxContainer);
		files_vb->set_v_size_flags(Control::SIZE_EXPAND_FILL);
		includes_vb->add_child(files_vb);

		files_vb->add_child(memnew(HSeparator));

		file_tree = memnew(Tree);
		file_tree->set_accessibility_name(TTRC("Available Options"));
		file_tree->set_v_scroll_enabled(false);
		file_tree->set_scroll_hint_mode(Tree::SCROLL_HINT_MODE_DISABLED);
		file_tree->set_theme_type_variation("TreeSecondary");
		file_tree->set_v_size_flags(Control::SIZE_EXPAND_FILL);
		file_tree->set_v_grow_direction(Control::GROW_DIRECTION_END);
		file_tree->connect("item_edited", callable_mp(this, &ProjectTemplate::_tree_item_edited));
		files_vb->add_child(file_tree);
	}

	// Other dialogs.

	fdialog = memnew(EditorFileDialog);
	fdialog->set_access(EditorFileDialog::ACCESS_FILESYSTEM);
	fdialog->connect("dir_selected", callable_mp(this, &ProjectTemplate::_dir_selected));
	fdialog->connect("canceled", callable_mp(this, &ProjectTemplate::show_dialog).bind(false), CONNECT_DEFERRED);
	add_child(fdialog);

	success_dialog = memnew(AcceptDialog);
	success_dialog->set_title(TTRC("Success"));
	success_dialog->set_text(TTRC("Template created successfully."));
	add_child(success_dialog);

	error_dialog = memnew(AcceptDialog);
	error_dialog->set_title(TTRC("Error"));
	error_dialog->set_autowrap(true);
	add_child(error_dialog);

	ScrollContainer *err_sc = memnew(ScrollContainer);
	error_dialog->add_child(err_sc);

	VBoxContainer *vb = memnew(VBoxContainer);
	err_sc->add_child(vb);

	error_label = memnew(Label);
	error_label->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
	vb->add_child(error_label);

	// Initialize file tree and HashMaps.

	script_ext["gd"] = false;
	script_ext["cs"] = false;

	file_type["script"] = false;
	file_type["scene"] = false;
	file_type["tres"] = false;

	_load_template_list(!is_editor);
	_create_tree();
}

ProjectTemplate::~ProjectTemplate() {
	singleton = nullptr;
}
