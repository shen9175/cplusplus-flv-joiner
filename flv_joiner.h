#pragma once

#define TAG_TYPE_METADATA 18


#define AMF_TYPE_NUMBER  0x00
#define AMF_TYPE_BOOLEAN  0x01
#define AMF_TYPE_STRING  0x02
#define AMF_TYPE_OBJECT  0x03
#define AMF_TYPE_MOVIECLIP  0x04
#define AMF_TYPE_NULL  0x05
#define AMF_TYPE_UNDEFINED  0x06
#define AMF_TYPE_REFERENCE  0x07
#define AMF_TYPE_MIXED_ARRAY  0x08
#define AMF_TYPE_END_OF_OBJECT  0x09
#define AMF_TYPE_ARRAY  0x0A
#define AMF_TYPE_DATE  0x0B
#define AMF_TYPE_LONG_STRING  0x0C
#define AMF_TYPE_UNSUPPORTED  0x0D
#define AMF_TYPE_RECORDSET  0x0E
#define AMF_TYPE_XML  0x0F
#define AMF_TYPE_CLASS_OBJECT  0x10
#define AMF_TYPE_AMF3_OBJECT  0x11


struct flv_header {
	uint8_t sig[3];
	uint8_t header_version;
	uint8_t type_flag;
	uint32_t  data_offset;
};
struct tag {
	uint32_t previous_tag_size;
	uint8_t data_type;
	uint32_t body_size;
	uint32_t timestamp;
	string body;
};

struct tag_return_type {
	//~tag_return_type();
	uint8_t type;
	void* pointer;
};

//unordered_map<string, tag_return_type> object;

class ECMAObject {
public:
	ECMAObject(uint32_t max) : max_number(max) {}
	~ECMAObject();
	void put(string k, tag_return_type v);
	tag_return_type get(string k) { return map.at(k); }
	void set(string k, tag_return_type v);
	vector<string> keys();
	uint32_t size() { return static_cast<uint32_t>(data.size()); }
public:
	uint32_t max_number;
	vector<pair<string, tag_return_type>> data;
	unordered_map<string, tag_return_type> map;
};

int32_t read_int(string& stream);
uint32_t read_uint(string& stream);
uint8_t read_byte(string& stream);
tag_return_type read_amf_number(string& stream);
tag_return_type read_amf_boolean(string& stream);
tag_return_type read_amf_string(string& stream);
tag_return_type read_amf_object(string& stream);
tag_return_type read_amf_mixed_array(string& stream);
tag_return_type read_amf_array(string& stream);
tag_return_type read_amf(string& stream);

void write_amf_number(string& stream, tag_return_type& tag);
void write_amf_boolean(string& stream, tag_return_type& tag);
void write_amf_string(string& stream, tag_return_type& tag);
void write_amf_string(string& stream, const string& s);
void write_amf_object(string& stream, tag_return_type& tag);
void write_amf_mixed_array(string& stream, tag_return_type& tag);
void write_amf_array(string& stream, tag_return_type& tag);
void write_amf(string& stream, tag_return_type& tag);
bool write_meta_tag(string& stream, pair<tag_return_type, tag_return_type> meta);
bool write_tag(string& stream, const tag& t);
pair<tag_return_type, tag_return_type> read_meta_data(string& stream);
pair<tag_return_type, tag_return_type> read_meta_tag(tag& item);
bool read_flv_header(string& stream, flv_header& header);
bool read_tag(string& stream, tag& stream_tag);


//void tag_return_type_destructor(tag_return_type o);
bool concat_flv(vector<string>& input, string& output);
