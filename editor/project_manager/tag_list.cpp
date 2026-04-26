/**************************************************************************/
/*  tag_list.cpp                                                          */
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

#include "tag_list.h"

#include "editor/file_system/editor_paths.h"

void TagList::_create_tag_config() {
	if (FileAccess::exists(_tag_config_path)) {
		_load_list();
		return;
	}

	_tag_config.set_value("tags", "config/tags", PackedStringArray());
	_save_config();
}

void TagList::add_tag(const String &p_tag) {
	append(p_tag);
	sort();
	_tag_config.load(_tag_config_path);
	_tag_config.set_value("tags", "config/tags", *this);

	_save_config();
}

void TagList::_load_list() {
	_tag_config.load(_tag_config_path);
	PackedStringArray tags = _tag_config.get_value("tags", "config/tags", PackedStringArray());
	append_array(tags);
	sort();
}

void TagList::_save_config() {
	_tag_config.save(_tag_config_path);
}

PackedStringArray TagList::get_tags() {
	return *this;
}

TagList::TagList() {
	_tag_config_path = EditorPaths::get_singleton()->get_data_dir().path_join("tags.cfg");
	_create_tag_config();
}
