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
#include "core/os/copymem.h"


#include "thirdparty/flatbuffers/include/flatbuffers/flexbuffers.h"

#include "core/io/file_access_pack.h"

#include "resource_importer_flexbuffer.h"

static const Variant flexbuffer_to_variant(flexbuffers::Reference buffer) {
	if (buffer.IsNull())
		return Variant();
	if (buffer.IsBool())
		return Variant(buffer.AsBool());
	if (buffer.IsInt())
		return Variant(buffer.AsInt64());
	if (buffer.IsUInt())
		return Variant(buffer.AsUInt64());
	if (buffer.IsFloat())
		return Variant(buffer.AsFloat());
	if (buffer.IsString())
		return Variant(buffer.AsString().c_str());
	if (buffer.IsVector()) {
		Array array;
		flexbuffers::Vector vector = buffer.AsVector();
		for (size_t i = 0; i < vector.size(); ++i) {
			array.append(flexbuffer_to_variant(vector[i]));
		}
		return Variant(array);
	}
	if (buffer.IsTypedVector()) {
		Array array;
		flexbuffers::TypedVector vector = buffer.AsTypedVector();
		if (vector[0].IsFloat()) {
			switch (vector.size()) {
				case 3: {
					real_t x = vector[0].AsDouble();
					real_t y = vector[1].AsDouble();
					real_t z = vector[2].AsDouble();

					Vector3 vector = Vector3(x, y, z);
					return Variant(vector);
				}
				case 4: {
					real_t x = vector[0].AsDouble();
					real_t y = vector[1].AsDouble();
					real_t z = vector[2].AsDouble();
					real_t w = vector[3].AsDouble();

					Quat quat = Quat(x, y, z, w);
					return Variant(quat);
				}
			}
		}
		for (size_t i = 0; i < vector.size(); ++i) {
			array.append(flexbuffer_to_variant(vector[i]));
		}
		return Variant(array);
	}
	if (buffer.IsMap()) {
		Dictionary dictionary;

		flexbuffers::Map map = buffer.AsMap();
		flexbuffers::TypedVector keys = map.Keys();
		flexbuffers::Vector values = map.Values();

		for (size_t i = 0; i < map.size(); ++i) {
			dictionary[keys[i].AsString().c_str()] = flexbuffer_to_variant(values[i]);
		}
		return Variant(dictionary);
	}

	return Variant();
}

static void flexbuffer_variant_add(flexbuffers::Builder &fbb, Variant variant) {
	switch (variant.get_type()) {
		case Variant::Type::NIL: {
			fbb.Null();
		} break;
		case Variant::Type::BOOL: {
			bool value = variant;
			fbb.Bool(value);
		} break;
		case Variant::Type::INT: {
			int value = variant;
			fbb.Int(value);
		} break;
		case Variant::Type::REAL: {
			double value = variant;
			fbb.Double(value);
		} break;
		case Variant::Type::STRING: {
			String value = variant;
			fbb.String(value.ascii().get_data());
		} break;
		case Variant::Type::VECTOR2: {
			Vector2 vector = variant;
			fbb.TypedVector([&]() {
				fbb.Double(vector.x);
				fbb.Double(vector.y);
			});
		} break;
		case Variant::Type::VECTOR3: {
			Vector3 vector = variant;
			fbb.TypedVector([&]() {
				fbb.Double(vector.x);
				fbb.Double(vector.y);
				fbb.Double(vector.z);
			});
		} break;
		case Variant::Type::QUAT: {
			Quat quat = variant;
			fbb.TypedVector([&]() {
				fbb.Double(quat.x);
				fbb.Double(quat.y);
				fbb.Double(quat.z);
				fbb.Double(quat.w);
			});
		} break;
		case Variant::Type::ARRAY: {
			Array array = variant;
			fbb.Vector([&]() {
				for (int i = 0; i < array.size(); ++i) {
					flexbuffer_variant_add(fbb, array[i]);
				}
			});
		} break;
		case Variant::Type::DICTIONARY: {
			Dictionary dictionary = variant;
			Array keys = dictionary.keys();
			Array values = dictionary.values();

			fbb.Map([&]() {
				for (int i = 0; i < dictionary.size(); ++i) {
					String key = keys[i];

					fbb.Key(key.ascii().get_data());
					flexbuffer_variant_add(fbb, values[i]);
				}
			});
		} break;
		default: {
		} break;
	}
}

static Vector<uint8_t> variant_to_flexbuffer(Variant variant) {
	flexbuffers::Builder fbb;
	flexbuffer_variant_add(fbb, variant);
	fbb.Finish();
	std::vector<uint8_t> std_vector = fbb.GetBuffer();
	Vector<uint8_t> godot_bytes;
	godot_bytes.resize(std_vector.size());
	copymem(godot_bytes.ptrw(), std_vector.data(), std_vector.size());
	return godot_bytes;
}

static Variant flexbuffer_to_variant(Vector<uint8_t> p_buffer) {
	std::vector<uint8_t> std_vector;
	std_vector.resize(p_buffer.size());
	copymem(std_vector.data(), p_buffer.ptr(), p_buffer.size());
	flexbuffers::Reference flat = flexbuffers::GetRoot(std_vector);
	return flexbuffer_to_variant(flat);
}

class FlexbuffersData : public Resource {
	GDCLASS(FlexbuffersData, Resource);
	Variant data;

protected:
	static void _bind_methods() {

		ClassDB::bind_method(D_METHOD("set_data", "data"), &FlexbuffersData::set_data);
		ClassDB::bind_method(D_METHOD("get_data"), &FlexbuffersData::get_data);
		ClassDB::bind_method(D_METHOD("set_flexbuffer", "json"), &FlexbuffersData::set_flexbuffers);
		ClassDB::bind_method(D_METHOD("get_flexbuffer"), &FlexbuffersData::get_flexbuffers);

		ADD_PROPERTY(PropertyInfo(Variant::NIL, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE | PROPERTY_USAGE_NIL_IS_VARIANT), "set_data", "get_data");
		ADD_PROPERTY(PropertyInfo(Variant::POOL_BYTE_ARRAY, "flexbuffer", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "set_flexbuffer", "get_flexbuffer");
	}

public:
	Variant get_data() const {
		return data;
	}
	void set_data(Variant p_data) {
		data = p_data;
	}
	Vector<uint8_t> get_flexbuffers() const {
		return variant_to_flexbuffer(data);
	}
	void set_flexbuffers(const Vector<uint8_t> p_buffer) {
		Variant new_data = flexbuffer_to_variant(p_buffer);
		set_data(new_data);
	}
	FlexbuffersData() {}
	~FlexbuffersData() {}
};

class ResourceImporterFlexbuffers : public ResourceImporter {
	GDCLASS(ResourceImporterFlexbuffers, ResourceImporter);

public:
	virtual String get_importer_name() const;
	virtual String get_visible_name() const;
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual String get_save_extension() const;
	virtual String get_resource_type() const;

	virtual int get_preset_count() const;
	virtual String get_preset_name(int p_idx) const;

	virtual void get_import_options(List<ImportOption> *r_options,
			int p_preset = 0) const;
	virtual bool
	get_option_visibility(const String &p_option,
			const Map<StringName, Variant> &p_options) const;
	virtual Error import(const String &p_source_file, const String &p_save_path,
			const Map<StringName, Variant> &p_options,
			List<String> *r_platform_variants,
			List<String> *r_gen_files = NULL,
			Variant *r_metadata = NULL);

	ResourceImporterFlexbuffers() {}
	~ResourceImporterFlexbuffers() {}
};
#endif // RESOURCE_IMPORTER_LOTTIE
