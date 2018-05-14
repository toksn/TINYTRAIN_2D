#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>

namespace tgf
{
	namespace json
	{
		static std::pair<std::size_t, std::size_t> findRange(const std::string& string, const char& start_token, const char& end_token)
		{
			std::size_t start = std::string::npos;
			std::size_t end = std::string::npos;

			

			start = string.find(start_token);
			if (start != std::string::npos)
			{
				std::size_t nested_counter = 1;
				end = start + 1;
				while (end < string.length() && nested_counter > 0)
				{
					if (string[end] == end_token)
						nested_counter--;
					else if (string[end] == start_token)
						nested_counter++;

					end++;
				}

				if (nested_counter > 0)
					end = std::string::npos;
			}

			return std::pair<std::size_t, std::size_t>(start, end);
		}


		static std::vector<std::size_t> findValidSeperators(const std::string& string, const char& seperator, std::size_t start = 0, std::size_t end = std::string::npos)
		{
			std::size_t nested_counter = 0;
			bool quotes = false;

			std::vector<std::size_t> indexes;

			if (end == std::string::npos || end > string.length())
				end = string.length();

			for (; start < end; start++)
			{
				if (string[start] == '{' || string[start] == '[')
					nested_counter++;
				else if (string[start] == '}' || string[start] == ']')
					nested_counter--;
				else if (string[start] == '"')
					quotes = !quotes;
				else if (string[start] == seperator && quotes == false && nested_counter == 0)
					indexes.push_back(start);
			}

			return indexes;
		}

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
			auto range = findRange(str, '{', '}');
			if(range.first != std::string::npos && range.second != std::string::npos)
			{
				std::size_t last = range.first + 1;
				auto seps = findValidSeperators(str, ',', range.first+1, range.second);

				// adding full range as another (fake)seperator to include the part after the last seperator to the end
				seps.push_back(range.second);
				for (auto& s : seps)
				{
					auto doublecolons = findValidSeperators(str, ':', last, s);
					// only use the first, in fact this should only return one index
					if (doublecolons.size())
					{
						std::size_t seperator = doublecolons[0];

						// found an object with a seperator. parse its contents.
						// start ----> "key" ---> seperator ---> value ---> end
						std::string keystring = str.substr(last, seperator - last);
						//std::string key;
						std::string datastring = str.substr(seperator + 1, s - seperator - 1);


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
									keystring = keystring.substr(namestart + 1, nameend - namestart - 1);
									// parse data
									obj[keystring] = json::data(datastring);
								}
							}
						}
					}
					last = s+1;
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
						auto range = findRange(data_string, '[', ']');
						if (range.first != std::string::npos && range.second != std::string::npos)
						{
							std::string sub = data_string.substr(range.first + 1, range.second - 1 - range.first);
							auto seps = findValidSeperators(sub, ',');
							std::size_t last = 0;

							// adding last str index as another (fake)seperator to include the part after the last seperator to the end
							if(sub.length() > 0)
								seps.push_back(sub.length() - 1);

							for (auto& s : seps)
							{
								std::string part = sub.substr(last, s - last);
								value.a.push_back(json::data(part));
								last = s + 1;
							}
							type = json::type::ARRAY;
						}
						break;
					}
					case '{':		// object
					{
						auto range = findRange(data_string, '{', '}');
						if (range.first != std::string::npos && range.second != std::string::npos)
						{
							std::string part = data_string.substr(range.first, range.second-range.first+1);
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
								try
								{									
									float f = std::stof(data_string);
									//int i = std::stoi(data_string);
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
										value.i = std::stoi(data_string);
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