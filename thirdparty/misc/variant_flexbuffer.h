/*
	Copyright 2020 Nova King (technobaboo)
	Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
	The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "messengermanager.hpp"
#include <Quat.hpp>
#include <String.hpp>
#include <Vector2.hpp>
#include <Vector3.hpp>

using namespace godot;

void MessengerManager::_register_methods() {
	register_method("send_signal", &MessengerManager::send_signal);
	register_method("execute_remote_method", &MessengerManager::execute_remote_method);
}

MessengerManager::MessengerManager() {}
MessengerManager::~MessengerManager() {
	delete this->messengerManager;
}

void MessengerManager::_init() {
	this->messengerManager = new StardustXR::MessengerManager(this);
}

void MessengerManager::send_signal(int clientID, String path, String method, Variant data) {
	std::vector<uint8_t> flexData = variantToFlexbuffer(data);
	this->messengerManager->messengers[clientID]->sendSignal(path.ascii().get_data(), method.ascii().get_data(), flexData);
}

void MessengerManager::execute_remote_method(int clientID, String remotePath, String remoteMethod, Variant args, String callbackPath, String callbackMethod, Array callbackArgsPrefix) {
	std::vector<uint8_t> flexData = variantToFlexbuffer(args);
	this->messengerManager->messengers[clientID]->executeRemoteMethod(remotePath.ascii().get_data(), remoteMethod.ascii().get_data(), flexData, [&](flexbuffers::Reference data) {
		callbackArgsPrefix.push_front(clientID);
		nodeMethodExecute(callbackArgsPrefix, callbackPath.ascii().get_data(), callbackMethod.ascii().get_data(), data, false);
	});
}

void MessengerManager::sendSignal(int sessionID, std::string path, std::string method, flexbuffers::Reference data) {
	Array prefix;
	prefix.append(sessionID);
	nodeMethodExecute(prefix, path, method, data, false);
}

std::vector<uint8_t> MessengerManager::executeMethod(int sessionID, std::string path, std::string method, flexbuffers::Reference args) {
	Array prefix;
	prefix.append(sessionID);
	return nodeMethodExecute(prefix, path, method, args, true);
}

std::vector<uint8_t> MessengerManager::nodeMethodExecute(Array prefix, std::string path, std::string method, flexbuffers::Reference args, bool returnValue) {
	Array array;
	Variant data = flexbufferToVariant(args);

	if (data.get_type() == Variant::Type::ARRAY) {
		array = flexbufferToVariant(args);
	} else {
		array.append(flexbufferToVariant(args));
	}
	while(prefix.size() > 0) {
		array.push_front(prefix.pop_back());
	}

	if(returnValue) {
		Variant returnVal = get_node(path.c_str() + 1)->callv(String(method.c_str()), array);
		return variantToFlexbuffer(get_node(path.c_str() + 1)->callv(String(method.c_str()), array));
	} else {
		get_node(path.c_str() + 1)->call_deferred(String(method.c_str()), array);
		return std::vector<uint8_t>();
	}
}

const Variant MessengerManager::flexbufferToVariant(flexbuffers::Reference buffer) {
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
			array.append(flexbufferToVariant(vector[i]));
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
			array.append(flexbufferToVariant(vector[i]));
		}
		return Variant(array);
	}
	if (buffer.IsMap()) {
		Dictionary dictionary;

		flexbuffers::Map map = buffer.AsMap();
		flexbuffers::TypedVector keys = map.Keys();
		flexbuffers::Vector values = map.Values();

		for(size_t i=0; i<map.size(); ++i) {
			dictionary[keys[i].AsString().c_str()] = flexbufferToVariant(values[i]);
		}
		return Variant(dictionary);
	}

	return Variant();
}

void MessengerManager::flexbufferVariantAdd(flexbuffers::Builder &fbb, Variant variant) {
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
					flexbufferVariantAdd(fbb, array[i]);
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
					flexbufferVariantAdd(fbb, values[i]);
				}
			});
		} break;
		default: {
		} break;
	}
}

std::vector<uint8_t> MessengerManager::variantToFlexbuffer(Variant variant) {
	flexbuffers::Builder fbb;
	flexbufferVariantAdd(fbb, variant);
	fbb.Finish();
	return fbb.GetBuffer();
}