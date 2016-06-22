#include "stdafx.h"



unordered_map<uint8_t, tag_return_type(*)(Stream& stream)> amf_readers = {
	{ AMF_TYPE_NUMBER, read_amf_number },
	{ AMF_TYPE_BOOLEAN, read_amf_boolean },
	{ AMF_TYPE_STRING, read_amf_string },
	{ AMF_TYPE_OBJECT, read_amf_object },
	{ AMF_TYPE_MIXED_ARRAY, read_amf_mixed_array },
	{ AMF_TYPE_ARRAY, read_amf_array }
};

unordered_map<uint8_t, void(*)(Stream& stream, tag_return_type& t)> amf_writers = {
	{ AMF_TYPE_NUMBER, write_amf_number },
	{ AMF_TYPE_BOOLEAN, write_amf_boolean },
	{ AMF_TYPE_STRING, write_amf_string },
	{ AMF_TYPE_OBJECT, write_amf_object },
	{ AMF_TYPE_MIXED_ARRAY, write_amf_mixed_array },
	{ AMF_TYPE_ARRAY, write_amf_array }
};
/*
tag_return_type::~tag_return_type() {
	switch (type) {
	case AMF_TYPE_NUMBER:
		delete reinterpret_cast<double*>(pointer);
		pointer = nullptr;
		break;
	case AMF_TYPE_BOOLEAN:
		delete reinterpret_cast<uint8_t*>(pointer);
		pointer = nullptr;
		break;
	case AMF_TYPE_STRING:
		delete[] reinterpret_cast<uint8_t*>(pointer);
		pointer = nullptr;
		break;
	case AMF_TYPE_OBJECT:
		for (auto &item : *(reinterpret_cast<unordered_map<string, tag_return_type>*>(pointer))) {
			delete reinterpret_cast<tag_return_type*>(item.second.pointer);
		}
		delete reinterpret_cast<unordered_map<string, tag_return_type>*>(pointer);
		pointer = nullptr;
		break;
	case AMF_TYPE_MIXED_ARRAY:
		delete reinterpret_cast<ECMAObject*>(pointer);
		pointer = nullptr;
		break;
	case AMF_TYPE_ARRAY:
		for (auto &item : *(reinterpret_cast<vector<tag_return_type>*>(pointer))) {
			delete reinterpret_cast<tag_return_type*>(item.pointer);
		}
		delete reinterpret_cast<vector<tag_return_type>*>(pointer);
		pointer = nullptr;
		break;
	default:
		break;
	}
}*/


Stream& Stream::operator=(const Stream& m) {
	stream = m.stream;
	start = m.start;
	end = m.end;
	return *this;
}
size_t Stream::size() const{
	assert(end >= start);
	return end - start;
}

void Stream::push_back(const char c) {
	assert(stream != nullptr);
	if (end + 1 > (*stream).size()) {
		(*stream).push_back(c);
		++end;
	} else {
		(*stream)[end++] = c;
	}
}

void Stream::append(const string& s) {
	
	//for (size_t i = 0; i < s.size(); ++i) {
	//	push_back(s[i]);
	//}
	assert(stream != nullptr);
	size_t len = s.size();
	size_t finalend = end + len;
	if (finalend > (*stream).size()) {
		(*stream).resize(finalend);
	}
	memcpy(&(*stream)[end], const_cast<char*>(&s[0]), len);
	end = finalend;
}
void Stream::append(const string& s, size_t len) {
	
	//for (size_t i = 0; i < len; ++i) {
	//	push_back(s[i]);
	//}
	assert(stream != nullptr);
	size_t finalend = end + len;
	if (finalend > (*stream).size()) {
		(*stream).resize(finalend);
	}
	memcpy(&(*stream)[end], const_cast<char*>(&s[0]), len);
	end = finalend;
}

void Stream::append(const Stream& s) {
	
	//for (size_t i = 0; i < input.size(); ++i) {
	//	push_back(input[i]);
	//}
	assert(stream != nullptr);
	size_t len = s.size();
	size_t finalend = end + len;
	if (finalend > (*stream).size()) {
		(*stream).resize(finalend);
	}
	memcpy(&(*stream)[end], const_cast<char*>(&s[0]), len);
	end = finalend;
}
void tag_return_type_destructor(tag_return_type o) {
	switch (o.type) {
	case AMF_TYPE_NUMBER:
		delete reinterpret_cast<double*>(o.pointer);
		o.pointer = nullptr;
		break;
	case AMF_TYPE_BOOLEAN:
		delete reinterpret_cast<uint8_t*>(o.pointer);
		o.pointer = nullptr;
		break;
	case AMF_TYPE_STRING:
		delete[] reinterpret_cast<uint8_t*>(o.pointer);
		o.pointer = nullptr;
		break;
	case AMF_TYPE_OBJECT:
		for (auto &item : *(reinterpret_cast<unordered_map<string, tag_return_type>*>(o.pointer))) {
			tag_return_type_destructor(item.second);
		}
		delete reinterpret_cast<unordered_map<string, tag_return_type>*>(o.pointer);
		o.pointer = nullptr;
		break;
	case AMF_TYPE_MIXED_ARRAY:
		delete reinterpret_cast<ECMAObject*>(o.pointer);
		o.pointer = nullptr;
		break;
	case AMF_TYPE_ARRAY:
		for (auto &item : *(reinterpret_cast<vector<tag_return_type>*>(o.pointer))) {
			tag_return_type_destructor(item);
		}
		delete reinterpret_cast<vector<tag_return_type>*>(o.pointer);
		o.pointer = nullptr;
		break;
	default:
		break;
	}
}
ECMAObject::~ECMAObject() {
	for (auto &item : data) {
		if (item.second.pointer) {
			tag_return_type_destructor(item.second);
		}
	}
}
void ECMAObject::put(string k, tag_return_type v) {
	if (map.find(k) == map.cend()) {
		data.push_back(make_pair(k, v));
		map[k] = v;
	} else {
		assert(0);
	}
}

void ECMAObject::set(string k, tag_return_type v) {
	bool flag = false;
	for (auto &item : data) {
		if (item.first == k) {
			if (item.second.pointer) {
				tag_return_type_destructor(item.second);
			}
			item.second = v;
			flag = true;
			break;
		}
	}
	assert(flag);
	map[k] = v;
}
vector<string> ECMAObject::keys() {
	vector<string> ret;
	for (auto item : data) {
		ret.push_back(item.first);
	}
	return ret;
}

int32_t read_int(Stream& stream) {
	assert(stream.size() >= 4);
	//int32_t ret = stream[0] << 24 | stream[1] << 16 | stream[2] << 8 | stream[3];//read big-endian
	int32_t ret;
	char bytes[4] = { stream[3],stream[2] ,stream[1] ,stream[0] };
	memcpy(&ret, bytes, sizeof(int32_t));
	stream = stream.substr(4);//pop first 4 bytes
	return ret;
}

uint32_t read_uint(Stream& stream) {
	assert(stream.size() >= 4);
	//uint32_t ret = stream[0] << 24 | stream[1] << 16 | stream[2] << 8 | stream[3];//read big-endian
	uint32_t ret;
	char bytes[4] = { stream[3],stream[2] ,stream[1] ,stream[0] };
	memcpy(&ret, bytes, sizeof(uint32_t));
	stream = stream.substr(4);//pop first 4 bytes
	return ret;
}
void write_uint(Stream& stream, uint32_t number) {
	char bytes[4];
	memcpy(bytes, &number, 4);
	stream.push_back(bytes[3]);
	stream.push_back(bytes[2]);
	stream.push_back(bytes[1]);
	stream.push_back(bytes[0]);
}

void write_byte(Stream& stream, char byte) {
	stream.push_back(byte);
}
uint8_t read_byte(Stream& stream) {
	assert(stream.size() >= 1);
	uint8_t ret = stream[0];
	stream = stream.substr(1);
	return ret;
}




tag_return_type read_amf_number(Stream& stream) {
	//In big endian, you store the most significant byte in the smallest address
	//In little endian, you store the least significant byte in the smallest address
	uint8_t buf[8] = {static_cast<uint8_t>(stream[7]), static_cast<uint8_t>(stream[6]), static_cast<uint8_t>(stream[5]), static_cast<uint8_t>(stream[4]), 
					  static_cast<uint8_t>(stream[3]), static_cast<uint8_t>(stream[2]), static_cast<uint8_t>(stream[1]), static_cast<uint8_t>(stream[0])};//big endian to little endian
	stream = stream.substr(8);
	tag_return_type ret;
	ret.type = AMF_TYPE_NUMBER;
	ret.pointer = new double;
	memcpy(ret.pointer, buf, sizeof(double));
	return ret;
}
tag_return_type read_amf_boolean(Stream& stream) {
	uint8_t b = read_byte(stream);
	assert(b == 0 || b == 1);
	tag_return_type ret;
	ret.type = AMF_TYPE_BOOLEAN;
	ret.pointer = new uint8_t;
	*(reinterpret_cast<uint8_t*>(ret.pointer)) = b;
	return ret;
}

tag_return_type read_amf_string(Stream& stream) {
	tag_return_type ret;
	ret.type = AMF_TYPE_STRING;
	if (stream.empty()) {
		ret.pointer = nullptr;
		return ret;
	}
	uint8_t twobytes[2] = { static_cast<uint8_t>(stream[1]),static_cast<uint8_t>(stream[0]) };
	stream = stream.substr(2);

	unsigned short len;
	memcpy(&len, twobytes, sizeof(unsigned short));
	assert(stream.size() >= len);
	ret.pointer = new uint8_t[len + 1];
	memcpy(ret.pointer, &stream[0], len);
	reinterpret_cast<uint8_t*>(ret.pointer)[len] = '\0';//to make sure when it is cast back, there is no trailing garbge.
	stream = stream.substr(len);
	return ret;
}
tag_return_type read_amf_object(Stream& stream) {
	tag_return_type ret;
	ret.type = AMF_TYPE_OBJECT;
	ret.pointer = new unordered_map<string, tag_return_type>;
	while (1) {
		tag_return_type k = read_amf_string(stream);
		if (strcmp(reinterpret_cast<char*>(k.pointer),"") == 0) {
			assert(read_byte(stream) == AMF_TYPE_END_OF_OBJECT);
			break;
		}
		tag_return_type v = read_amf(stream);
		string key = reinterpret_cast<char*>(k.pointer);
		unordered_map<string, tag_return_type>* temp = reinterpret_cast<unordered_map<string, tag_return_type>*>(ret.pointer);
		if (temp->find(key) == temp->cend()) {
			(*temp)[key] = v;
		} else {
			assert(0);
		}
		
		delete []reinterpret_cast<uint8_t*>(k.pointer);
		k.pointer = nullptr;
	}
	return ret;
}
tag_return_type read_amf_mixed_array(Stream& stream) {
	tag_return_type ret;
	ret.type = AMF_TYPE_MIXED_ARRAY;
	uint32_t max_number = read_uint(stream);
	ECMAObject* mix_results = new ECMAObject(max_number);
	while (1) {
		tag_return_type k = read_amf_string(stream);
		if (k.pointer == nullptr) { //dirty fix for the invalid Qiyi flv
			break;
		}
		if (strcmp(reinterpret_cast<char*>(k.pointer),"") == 0) {
			assert(read_byte(stream) == AMF_TYPE_END_OF_OBJECT);
			break;
		}
		tag_return_type v = read_amf(stream);
		string key = reinterpret_cast<char*>(k.pointer);
		mix_results->put(key, v);
		delete []reinterpret_cast<uint8_t*>(k.pointer);
		k.pointer = nullptr;
	}
	assert(mix_results->size() == max_number);
	ret.pointer = mix_results;
	return ret;
}
tag_return_type read_amf_array(Stream& stream) {
	uint32_t n = read_uint(stream);
	tag_return_type ret;
	ret.type = AMF_TYPE_ARRAY;
	ret.pointer = new vector<tag_return_type>;
	for (uint32_t i = 0; i < n; ++i) {
		tag_return_type v = read_amf(stream);
		reinterpret_cast<vector<tag_return_type>*>(ret.pointer)->push_back(v);
	}
	return ret;
}

tag_return_type read_amf(Stream& stream) {
	return amf_readers[read_byte(stream)](stream);
}
void write_amf_number(Stream& stream, tag_return_type& tag) {
	assert(tag.type == AMF_TYPE_NUMBER);
	char bytes[8];
	memcpy(bytes, tag.pointer, sizeof(double));
	for (int i = 7; i >= 0; --i) {
		stream.push_back(bytes[i]);
	}
	delete reinterpret_cast<double*>(tag.pointer);
	tag.pointer = nullptr;
}
void write_amf_boolean(Stream& stream, tag_return_type& tag) {
	assert(tag.type == AMF_TYPE_BOOLEAN);
	if (*reinterpret_cast<uint8_t*>(tag.pointer)) {
		write_byte(stream, '\x01');
	} else {
		write_byte(stream, '\x00');
	}
	delete reinterpret_cast<uint8_t*>(tag.pointer);
	tag.pointer = nullptr;
}
void write_amf_string(Stream& stream, tag_return_type& tag) {
	assert(tag.type == AMF_TYPE_STRING);
	char* s = reinterpret_cast<char*>(tag.pointer);
	char bytes[2];
	size_t len_t = strlen(s);
	assert(len_t <= 0xff);
	unsigned short len = static_cast<unsigned short>(len_t);
	memcpy(bytes, &len, sizeof(unsigned short));
	stream.push_back(bytes[1]);
	stream.push_back(bytes[0]);
	stream.append(s,len);
	delete[] reinterpret_cast<uint8_t*>(tag.pointer);
	tag.pointer = nullptr;
}
void write_amf_string(Stream& stream, const string& s) {
	char bytes[2];
	size_t len_t = s.length();
	assert(len_t <= 0xff);
	unsigned short len = static_cast<unsigned short>(len_t);
	memcpy(bytes, &len, sizeof(unsigned short));
	stream.push_back(bytes[1]);
	stream.push_back(bytes[0]);
	stream.append(s);
}
void write_amf_object(Stream& stream, tag_return_type& tag) {
	assert(tag.type == AMF_TYPE_OBJECT);
	//the order of k matters ?
	for (auto &item : *(reinterpret_cast<unordered_map<string, tag_return_type>*>(tag.pointer))) {
		write_amf_string(stream, item.first);
		write_amf(stream, item.second);
	}
	write_amf_string(stream, "");
	write_byte(stream, AMF_TYPE_END_OF_OBJECT);
	delete reinterpret_cast<unordered_map<string, tag_return_type>*>(tag.pointer);
	tag.pointer = nullptr;
}
void write_amf_mixed_array(Stream& stream, tag_return_type& tag) {
	assert(tag.type == AMF_TYPE_MIXED_ARRAY);
	uint32_t max_number = reinterpret_cast<ECMAObject*>(tag.pointer)->max_number;
	for (auto &item : reinterpret_cast<ECMAObject*>(tag.pointer)->data) {//&item has to pass by reference, write_amf will release the heap and let the pointer = nullptr, then it will not be release again in destructor
		write_amf_string(stream, item.first);
		write_amf(stream, item.second);
	}
	write_amf_string(stream, "");
	write_byte(stream, AMF_TYPE_END_OF_OBJECT);
	delete reinterpret_cast<ECMAObject*>(tag.pointer);
	tag.pointer = nullptr;
}
void write_amf_array(Stream& stream, tag_return_type& tag) {
	assert(tag.type == AMF_TYPE_ARRAY);
	vector<tag_return_type>* v = reinterpret_cast<vector<tag_return_type>*>(tag.pointer);
	assert(v->size() <= 0xffff);
	write_uint(stream, static_cast<uint32_t>(v->size()));
	for (auto i = 0; i < v->size(); ++i) {
		write_amf(stream, (*v)[i]);
	}
	delete reinterpret_cast<vector<tag_return_type>*>(tag.pointer);
	tag.pointer = nullptr;
}
void write_amf(Stream& stream, tag_return_type& tag) {
	write_byte(stream, tag.type);
	amf_writers[tag.type](stream, tag);
}

pair<tag_return_type, tag_return_type> read_meta_data(Stream& stream) {
	tag_return_type	meta_type = read_amf(stream);
	tag_return_type meta = read_amf(stream);
	return{ meta_type, meta };
}

pair<tag_return_type, tag_return_type> read_meta_tag(tag& item) {
	assert(item.data_type == TAG_TYPE_METADATA);
	assert(item.timestamp == 0);
	assert(item.previous_tag_size == 0);
	return read_meta_data(item.body);
}

bool write_meta_tag(Stream& stream, pair<tag_return_type, tag_return_type> meta) {
	Stream buffer(stream);
	write_amf(buffer, meta.first);//meta_type
	write_amf(buffer, meta.second);//meta_data
	tag t;
	t.data_type = TAG_TYPE_METADATA;
	t.timestamp = 0;
	t.body_size = static_cast<uint32_t>(buffer.size());
	t.body = buffer;
	t.previous_tag_size = 0;
	write_tag(stream, t);
	return true;
}

bool read_flv_header(Stream& stream, flv_header& header) {
	header.sig[0] = read_byte(stream);
	header.sig[1] = read_byte(stream);
	header.sig[2] = read_byte(stream);
	if (header.sig[0] != 'F' || header.sig[1] != 'L' || header.sig[2] != 'V') {
		return false;
	}
	header.header_version = read_byte(stream);
	if (header.header_version != 1) {
		return false;
	}
	header.type_flag = read_byte(stream);
	if (header.type_flag != 5) {
		return false;
	}
	header.data_offset = read_uint(stream);
	if (header.data_offset != 9) {
		return false;
	}
	return true;
}
void write_flv_header(Stream& stream) {
	stream.push_back('F');
	stream.push_back('L');
	stream.push_back('V');
	write_byte(stream, 1);
	write_byte(stream, 5);
	write_uint(stream, 9);
}
bool read_tag(Stream& stream, tag& stream_tag) {
	//cout << stream.size() << endl;
	if (stream.size() == 4) {
		return false;
	}
	stream_tag.previous_tag_size = read_uint(stream);
	stream_tag.data_type = read_byte(stream);
	//stream_tag.body_size = (stream[0] << 16 | stream[1] << 8 | stream[2]);
	//overflow in this way:  2<<8-->0 in char, 0 << 16 | 2 << 8 | = 0, and 0|-112 = -112->uint32_t -> inverse 1110000-> 11111111111111111111111110001111 = 4294967183
	uint32_t a0 = static_cast<uint8_t>(stream[0]) << 16;
	uint32_t a1 = static_cast<uint8_t>(stream[1]) << 8;
	uint32_t a2 = static_cast<uint8_t>(stream[2]);
	stream_tag.body_size = a0 | a1 | a2;
	//stream_tag.body_size = static_cast<uint32_t>(stream[0] << 16 | stream[1] << 8 | stream[2]);
	assert(stream_tag.body_size < 1024 * 1024 * 128);
	stream = stream.substr(3);
	assert(stream.size() >= 4);
	a0 = static_cast<uint8_t>(stream[0]) << 16;
	a1 = static_cast<uint8_t>(stream[1]) << 8;
	a2 = static_cast<uint8_t>(stream[2]);
	stream_tag.timestamp = static_cast<uint8_t>(stream[3]) + (a0 | a1 | a2);
	//stream_tag.timestamp = static_cast<uint8_t>(stream[3]) + static_cast<uint32_t>(stream[0] << 16 | stream[1] << 8 | stream[2]);
	stream = stream.substr(4);
	assert(stream.size() >= 3);
	assert(stream[0] == 0 && stream[1] == 0 && stream[2] == 0);
	stream = stream.substr(3);
	stream_tag.body = stream.substr(0,stream_tag.body_size);
	stream = stream.substr(stream_tag.body_size);
	return true;
}

bool write_tag(Stream& stream, const tag& t) {
	write_uint(stream, t.previous_tag_size);
	write_byte(stream, t.data_type);
	write_byte(stream, t.body_size >> 16 & 0xff);
	write_byte(stream, t.body_size >> 8 & 0xff);
	write_byte(stream, t.body_size & 0xff);
	write_byte(stream, t.timestamp >> 16 & 0xff);
	write_byte(stream, t.timestamp >> 8 & 0xff);
	write_byte(stream, t.timestamp & 0xff);
	write_byte(stream, t.timestamp >> 24 & 0xff);
	stream.push_back('\0');
	stream.push_back('\0');
	stream.push_back('\0');
	stream.append(t.body);
	return true;
}

bool concat_flv(vector<string>& input, string& output) {
	vector<tag> meta_tags(input.size());//assume->not sure->may it is-> every flv file only have one meta-tag and it is always in the begining.
	vector<pair<tag_return_type, tag_return_type>> metatypes_metas;
	vector<Stream> streamin(input.size());
	//unordered_set<tag_return_type> metatypes;
	double total_duration = 0.0f;
	for (size_t i = 0; i < input.size(); ++i) {
		flv_header header;
		Stream s(&input[i]);
		streamin[i] = s;
		//output += input[i];
		read_flv_header(streamin[i], header);
		read_tag(streamin[i], meta_tags[i]);
		pair<tag_return_type, tag_return_type> metatype_meta = read_meta_tag(meta_tags[i]);
		metatypes_metas.push_back(metatype_meta);
		//metatypes.insert(metatype_meta.first);
		assert(metatype_meta.second.type == AMF_TYPE_MIXED_ARRAY);
		assert(reinterpret_cast<ECMAObject*>(metatype_meta.second.pointer)->get("duration").type == AMF_TYPE_NUMBER);
		total_duration += *(reinterpret_cast<double*>(reinterpret_cast<ECMAObject*>(metatype_meta.second.pointer)->get("duration").pointer));
	}
	//assert(metatypes.size() == 1);
	tag_return_type v;
	v.type = AMF_TYPE_NUMBER;
	v.pointer = new double;
	*reinterpret_cast<double*>(v.pointer) = total_duration;
	reinterpret_cast<ECMAObject*>(metatypes_metas[0].second.pointer)->set("duration", v);
	output = "";
	Stream out(&output);
	out.clear();
	write_flv_header(out);
	write_meta_tag(out, metatypes_metas[0]);
	uint32_t timestamp_start = 0;
	uint32_t record_last_previous_tag_size;
	for (size_t i = 0; i < input.size(); ++i) {
		tag temp;
		uint32_t timestamp;
		while (read_tag(streamin[i], temp)) {
			temp.timestamp += timestamp_start;//combining timestamp with other previous file
			timestamp = temp.timestamp;//record the last tag timestamp of this combined file
			record_last_previous_tag_size = temp.previous_tag_size;
			write_tag(out, temp);
		}
		timestamp_start = timestamp;
	}
	write_uint(out, record_last_previous_tag_size);
	/*
	cout << out.size() << endl;
	cout << output.size() << endl;
	if (out.size() < output.size()) {
		output = output.substr(0, out.size());
	}
	*/
	return true;
}