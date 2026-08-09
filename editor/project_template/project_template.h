/**************************************************************************/
/*  project_template.h                                                    */
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

#pragma once

#include "core/io/config_file.h"
#include "scene/gui/dialogs.h"

class Button;
class CheckBox;
class EditorFileDialog;
class Label;
class LineEdit;
class TextEdit;
class TextureRect;
class Tree;
class TreeItem;

class ProjectTemplateItem : public HBoxContainer {
	GDCLASS(ProjectTemplateItem, HBoxContainer);

	bool icon_loaded = false;
	bool is_hovering = false;
	bool is_selected = false;
	bool is_focus_hidden = false;

	Label *title_label = nullptr;
	Button *open_explorer_btn = nullptr;
	Button *edit_btn = nullptr;
	Button *remove_btn = nullptr;
	Button *delete_btn = nullptr;

	String old_title;
	String old_description;
	String template_dir;

	void _accessibility_action_scroll_into_view(const Variant &p_data);
	void _accessibility_action_focus(const Variant &p_data);
	void _accessibility_action_blur(const Variant &p_data);

	void _open_template_folder();
	void _set_template_icon();

protected:
	void _notification(int p_what);
	void gui_input(const Ref<InputEvent> &p_gui_input) override;

public:
	String title;
	String description;

	TextureRect *icon = nullptr;

	void set_selected(bool p_selected, bool p_hide_focus = false);
	void set_template(const String &p_dir);
	String get_template();

	void connect_edit_button(const Callable p_callable);
	void connect_remove_button(const Callable p_callable);
	void connect_delete_button(const Callable p_callable);

	void update_title(const String &p_title, bool p_revert = false);
	void update_description(const String &p_description, bool p_revert = false);
	void save_changes();
	void revert_changes();

	ProjectTemplateItem(const String &p_title, const String &p_description = String());
};

class ProjectTemplate : public Control {
	GDCLASS(ProjectTemplate, Control);

	friend class ProjectTemplateItem;

	static ProjectTemplate *singleton;

	bool is_editor = false; // True if in the editor, false if in the project manager.

	struct ProjectTemplateComparator {
		_FORCE_INLINE_ bool operator()(const ProjectTemplateItem *a, const ProjectTemplateItem *b) const {
			int order = a->title.naturalnocasecmp_to(b->title);

			return order < 1;
		}
	};

	// Enums
public:
	enum Mode {
		MODE_MANAGE,
		MODE_CREATE,
	};

private:
	enum MessageType {
		MESSAGE_ERROR,
		MESSAGE_WARNING,
		MESSAGE_SUCCESS,
	};

	enum InputType {
		NAME,
		PATH,
	};

	Mode mode = MODE_MANAGE;

	/* Directory Management */

	// Directory scan.

	struct ScanData {
		Thread *thread = nullptr;
		PackedStringArray paths_to_scan;
		List<String> dir_items;
		SafeFlag scan_in_progress;
	};
	ScanData *scan_data = nullptr;
	AcceptDialog *scan_progress = nullptr;

	static void _scan_thread(void *p_scan_data);
	void _scan_finished();
	static void _scan_dir_recursive(const String &p_path, List<String> *r_dirs, const SafeFlag &p_scan_active);

	void _scan_project_dir();
	void _cleanup_previous_scan();

	// Directory operations.

	String templates_dir_setting = "filesystem/directories/project_template/project_templates_folder";
	String project_dir;
	String templates_dir;
	String template_folder;
	String scanned_dir;

	bool template_dirs_exist = false;

	void _set_templates_dir(const String p_dir);
	void _dir_selected(const String &p_dir);

	void _create_templates_dir();
	Error _create_folders();
	PackedStringArray _copy_files_all();
	PackedStringArray _copy_files_individual(const PackedStringArray &p_from, const String &p_to, const String &p_filter = String());
	PackedStringArray _copy_script_files_individual(const PackedStringArray &p_from, const String &p_to, const String &p_filter);

	/* Template Management */

	// Template members.

	int naming_convention = 0;
	String default_name = "New Project Template";
	HashMap<String, bool> template_names;
	Vector<PackedStringArray> template_name_archive;

	Vector<ProjectTemplateItem *> template_items;
	Vector<ProjectTemplateItem *> edited_templates;
	Vector<ProjectTemplateItem *> remove_templates;
	Vector<ProjectTemplateItem *> delete_templates;

	// Template creation management.

	bool status_red = false;

	void _set_name(const String p_text, bool p_edit);
	void _set_path(const String p_path);
	void _set_message(const String &p_msg, MessageType p_type, InputType p_input_type, bool p_edit = false);
	void _update_dialog_ok_button();

	// Manage project templates.

	ConfirmationDialog *manage_dialog = nullptr;
	VBoxContainer *templates_container = nullptr;
	Button *add_btn = nullptr;
	Label *no_templates_label = nullptr;

	AcceptDialog *delete_dialog = nullptr;

	void _load_template_list(bool p_load_template_items);
	void _update_template_list(const String &p_template);
	void _manage_templates();
	void _apply_template_changes(bool p_canceled = false);
	void _remove_template(ProjectTemplateItem *p_item);
	void _delete_template(ProjectTemplateItem *p_item);
	void _delete_template_folder(ProjectTemplateItem *p_item);
	void _show_delete_template_warning_dialog(ProjectTemplateItem *p_item);

	void select_template_item(int p_index, bool p_hide_focus = false);

	// Edit project template.

	ConfirmationDialog *edit_dialog = nullptr;
	TextureRect *title_status_rect = nullptr;
	Label *title_status_label = nullptr;
	LineEdit *title_edit = nullptr;
	TextEdit *desc_edit = nullptr;

	void _edit_template(ProjectTemplateItem *p_item);
	void _edit_template_confirmed();

	// Create project template.

	ConfirmationDialog *create_dialog = nullptr;
	VBoxContainer *name_container = nullptr;
	LineEdit *name_edit = nullptr;
	Label *name_status_label = nullptr;
	TextureRect *name_status_rect = nullptr;

	VBoxContainer *path_container = nullptr;
	LineEdit *path_edit = nullptr;
	Label *path_status_label = nullptr;
	TextureRect *path_status_rect = nullptr;
	Button *browse_btn = nullptr;

	TextEdit *template_desc = nullptr;

	void _open_create_dialog(bool p_reset_name = true);
	bool _get_changed_settings();

	void _create_template();
	void _copy_files_to_template_dir();
	void _cleanup_template_files_and_dirs();

	// Project creation.

	Error _copy_template_settings();

	// Other dialogs.

	EditorFileDialog *fdialog = nullptr;

	AcceptDialog *error_dialog = nullptr;
	Label *error_label = nullptr;

	AcceptDialog *success_dialog = nullptr;

	void _show_file_dialog();
	void _show_error(const String &p_error, Size2i p_minsize = Size2i());

	// Template includes.

	HashMap<String, bool> script_ext;
	HashMap<String, bool> file_type;

	PackedStringArray folders;
	PackedStringArray script_files;
	PackedStringArray scene_files;
	PackedStringArray tres_files;

	CheckBox *settings_cb = nullptr;
	CheckBox *folders_cb = nullptr;
	CheckBox *files_cb = nullptr;
	Tree *file_tree = nullptr;
	VBoxContainer *includes_vb = nullptr;
	VBoxContainer *files_vb = nullptr;

	TreeItem *all_include = nullptr;
	TreeItem *script_include = nullptr;
	TreeItem *scene_include = nullptr;
	TreeItem *tres_include = nullptr;

	HashMap<TreeItem *, bool> file_tree_items;

	void _create_tree();
	TreeItem *create_tree_item(const String &p_item_text, TreeItem *p_parent);
	void _update_tree_items();
	void _update_tree_item(TreeItem *p_item, TreeItem *p_parent, bool p_add);
	void _tree_item_edited();
	void _update_possible_includes();
	void _update_file_includes();

	// Config.

	ConfigFile template_config;
	ConfigFile file_config;

	void _save_folder_config();
	void _save_file_config();
	void _save_template_config();
	void update_templates_quick_access_config(const String &p_template, bool p_remove_template_key, bool p_load_template = false);
	bool templates_quick_access_config_has_key(const String &p_key);

protected:
	void _notification(int p_what);

public:
	static ProjectTemplate *get_singleton() { return singleton; }
	static void initialize(bool p_is_editor);

	//  Directory operations.

	void set_project_path(const String &p_path);

	// Project creation.

	bool template_valid = false;

	void template_selected(const String &p_project_path, const String &p_template);
	void copy_to_project_dir();

	// Template dialog.

	void set_mode(Mode p_mode);
	void show_dialog(bool p_rest_name = true);

	ProjectTemplate(bool p_is_editor);
	~ProjectTemplate();
};
