/*************************************************************************/
/*  resource_importer_flexbuffer.h                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

/*
	Copyright 2020 Nova King (technobaboo)
	Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
	The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef RESOURCE_IMPORTER_FLEXBUFFER
#define RESOURCE_IMPORTER_FLEXBUFFER

#include "core/io/resource_importer.h"
#include "core/io/resource_saver.h"

#include "thirdparty/flatbuffers/include/flatbuffers/flexbuffers.h"

#include "core/io/file_access_pack.h"

#include "resource_importer_flexbuffer.h"

static const Variant flatbuffer_to_variant(flexbuffers::Reference buffer);

static void flatbuffer_variant_add(flexbuffers::Builder &fbb, Variant variant);

static Vector<uint8_t> variant_to_flatbuffer(Variant variant);

static Variant flatbuffer_buffer_to_variant(Vector<uint8_t> p_buffer);

class FlatbuffersData : public Resource {
	GDCLASS(FlatbuffersData, Resource);
	Variant data;

protected:
	static void _bind_methods();

public:
	Variant get_data() const;
	void set_data(Variant p_data);
	Vector<uint8_t> get_flatbuffers() const;
	void set_flatbuffers(const Vector<uint8_t> p_buffer);
	FlatbuffersData() {}
	~FlatbuffersData() {}
};

class ResourceImporterFlatbuffers : public ResourceImporter {
	GDCLASS(ResourceImporterFlatbuffers, ResourceImporter);

public:
	virtual String get_importer_name() const override;
	virtual String get_visible_name() const override;
	virtual void get_recognized_extensions(List<String> *p_extensions) const override;
	virtual String get_save_extension() const override;
	virtual String get_resource_type() const override;

	virtual int get_preset_count() const override;
	virtual String get_preset_name(int p_idx) const override;
	virtual void get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset = 0) const override;

	virtual bool get_option_visibility(const String &p_path, const String &p_option, const Map<StringName, Variant> &p_options) const override;
	virtual Error import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files = nullptr, Variant *r_metadata = nullptr) override;

	ResourceImporterFlatbuffers() {}
	~ResourceImporterFlatbuffers() {}
};
#endif // RESOURCE_IMPORTER_LOTTIE
