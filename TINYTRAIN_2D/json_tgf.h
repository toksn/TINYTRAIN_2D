#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>

namespace tgf
{
	namespace json
	{
		enum class type
		{
			BOOLEAN,
			INTEGER,
			FLOAT,
			STRING,
			OBJECT,
			ARRAY,
			NULL_OBJ
		};

		class data;
		typedef std::map<std::string, json::data> object;

		class data
		{				
		public:
			data()
			{
				type = json::type::NULL_OBJ;
			}
			data(const data& other) 
			{ 
				value = other.value;
				type = other.type;
			}
			data(std::string& data_string);

			~data() { };

			json::type type = type::NULL_OBJ;

			bool* as_bool()						{ return type == json::type::BOOLEAN ? &(value.b) : nullptr; }
			int* as_int()						{ return type == json::type::INTEGER ? &(value.i) : nullptr; }
			float* as_float()					{ return type == json::type::FLOAT	 ? &(value.f) : nullptr; }
			std::string* as_string()			{ return type == json::type::STRING  ? &(value.s) : nullptr; }
			json::object* as_object()			{ return type == json::type::OBJECT  ? &(value.o) : nullptr; }
			std::vector<json::data>* as_array() { return type == json::type::ARRAY	 ? &(value.a) : nullptr; }

			//std::string toString();

		//private:
			struct val
			{
				bool b;
				int i;
				float f;
				std::string s;
				json::object o;
				std::vector<json::data> a;
				//void* p;
			} value;
		};

		static json::object parseFromJsonString(std::string str)
		{
			json::object obj;

			std::size_t start = str.find('{');
			if (start != std::string::npos)
			{
				//std::size_t end = str.find('}');
				// todo check for new start 
				std::size_t anotherstart = start;
				std::size_t end = start;
				while (anotherstart != std::string::npos)
				{
					anotherstart = end + 1;
					end = str.find('}', end + 1);

					if (end != std::string::npos)
					{
						std::string temp = str.substr(anotherstart, end - anotherstart);
						anotherstart = temp.find('{');
					}
					else
						anotherstart = std::string::npos;
				}
				
				if (end != std::string::npos)
				{
					std::size_t seperator = str.find(':', start + 1);
					if (seperator != std::string::npos && seperator < end)
					{
						// found an object with a seperator. parse its contents.
						// start ----> "key" ---> seperator ---> value ---> end
						std::string keystring = str.substr(start+1, seperator-start-1);
						//std::string key;
						std::string datastring = str.substr(seperator+1, end-seperator-1);


						if (datastring.length())
						{
							std::size_t namestart = keystring.find('"');
							std::size_t nameend = std::string::npos;
							if (namestart != std::string::npos)
							{
								nameend = keystring.find('"', namestart + 1);
								if (nameend != std::string::npos && nameend - namestart > 1)
								{
									// found a key name within doublequotes
									keystring = keystring.substr(namestart+1, nameend - namestart - 1);
									// parse data
									obj[keystring] = json::data(datastring);
								}
							}
						}
					}
				}
			}
			return obj;
		}

		static std::string parseToJsonString(json::object obj)
		{
			return std::string("todo: parseToJsonString(...) not implemented yet");
		}

		data::data(std::string& data_string)
		{
			//json::data dat;
			
			
			std::size_t start = data_string.find_first_not_of(" \n\t");
			if (start != std::string::npos)
			{
				switch (data_string[start])
				{		
					// recursive when array or object
					case '[':		// array
					{
						std::size_t anotherstart = 0;
						std::size_t end = start;
						while (anotherstart != std::string::npos)
						{
							end = data_string.find(']', end+1);
							if (end != std::string::npos)
								anotherstart = data_string.find("[", anotherstart, end - anotherstart);
							else
								anotherstart = std::string::npos;
						}
							
						
						

						if (end != std::string::npos)
						{
							char seperator = ',';
							std::size_t last = start+1;
							std::size_t pos = data_string.find(seperator, last);
							while (pos < end)
							{
								std::string part = data_string.substr(last, pos - 1 - last);
								value.a.push_back(json::data(part));

								last = pos + 1;
								pos = data_string.find(seperator, last);
							} 
							// fill in the part from last seperator to end
							std::string part = data_string.substr(last, end - 1 - last);
							value.a.push_back(json::data(part));

							type = json::type::ARRAY;
						}
						break;
					}
					case '{':		// object
					{
						//std::size_t end = data_string.find('}', start + 1);
						
						// todo check for another start
						std::size_t anotherstart = 0;
						std::size_t end = start;
						while (anotherstart != std::string::npos)
						{
							end = data_string.find('}', end + 1);
							if (end != std::string::npos)
								anotherstart = data_string.find("{", anotherstart, end - anotherstart);
							else
								anotherstart = std::string::npos;
						}

						
						if (end != std::string::npos)
						{
							std::string part = data_string.substr(start, end-start+1);
							value.o = parseFromJsonString(part);

							type = json::type::OBJECT;
						}
						break;
					}
					// rest is direct initialization
					case '"':		// string
					{
						std::size_t end = data_string.find('"', start + 1);
						if (end != std::string::npos)
						{
							value.s = data_string.substr(start + 1, end - 1 - start);
							type = json::type::STRING;
						}
						break;
					}
					// check for "true" or "false", check for numbers, ortherwise nothing when null, empty or invalid
					default:
					{
						std::size_t end = data_string.find("true", start);
						if (end != std::string::npos)
						{
							value.b = true;
							type = json::type::BOOLEAN;
						}
						else
						{
							end = data_string.find("false", start);
							if (end != std::string::npos)
							{
								value.b = false;
								type = json::type::BOOLEAN;
							}
							else
							{
								std::string sub = data_string.substr(start + 1);
								try
								{									
									float f = std::stof(sub);
									//int i = std::stoi(data_string.substr(start + 1));
									if (f == (int)f)
									{
										value.i = f;
										type = json::type::INTEGER;
									}
									else
									{
										value.f = f;
										type = json::type::FLOAT;
									}
								}
								catch (const std::exception&)
								{
									try
									{
										value.i = std::stoi(sub);
										type = json::type::INTEGER;
									}
									catch (const std::exception&)
									{
										type = json::type::NULL_OBJ;
									}
								}
							}
						}

						break;
					}
				}
			}
		}

		/*struct object
		{
			std::map<std::string, json::data> enties;
			const json::data& operator [](std::string key)
			{
				return enties.at(key);
			}

			std::string toString();
		};*/
	}
}