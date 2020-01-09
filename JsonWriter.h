#ifndef JsonWriterH
#define JsonWriterH
#include <vector>
#include <unordered_map>
#include <memory>

namespace Json
{
	class Node;
	template<typename T>
	class Value;
	class Object;
	template<typename T>
	class Array;

	class Node {
	public:
		virtual std::ostream& write(std::ostream& os) const noexcept = 0;

		friend std::ostream& operator<<(std::ostream& os, const Node& node) {
			return node.write(os);
		}

		virtual ~Node() {};

		template<typename T>
		static std::shared_ptr<Node> create(const T& rval);

		static std::shared_ptr<Node> create(const Object& rval);

		static std::shared_ptr<Node> create(const char *rval);

		template<typename T>
		static std::shared_ptr<Node> create(const std::initializer_list<T>& value);

		template<typename T>
		static std::shared_ptr<Node> create(const std::vector<T>& value);
	};


	//------------WriterImpl---------------//
	template<typename T>
	inline std::ostream& writeImpl(std::ostream& os, const T& value) {
		return os << value;
	}

	inline std::ostream& writeImpl(std::ostream& os, const std::tm& value) {
		return os << '\"' << std::put_time(&value, "%Y-%m-%dT%H:%M:%S") << '\"';
	}

	std::ostream& writeImpl(std::ostream& os, const std::string& value) {
		std::ostringstream escaped;
		for (char ch : value) {
			switch (ch) {
			case '\"':
			case '\\':
			case '/':
				escaped << '\\' << ch;
				break;
			default:
				escaped << ch;
			}
		}

		return os << '\"' << escaped.str() << '\"';
	}


	template<typename T>
	class Array : public Node {
	private:
		std::vector<T> children;
	public:
		Array(const std::vector<T>& children)
			:children(children) {}

		Array& operator()(T&& val) {
			children.push_back(std::move(val));
			return *this;
		}

		virtual std::ostream& write(std::ostream& os) const noexcept override {
			os << '[';
			for (auto it = children.begin(); it != children.end(); ++it) {
				writeImpl(os, *it);
				if (std::next(it) != children.end())
					os << ',';
			}
			os << ']';

			return os;
		}
	};

	template<typename T>
	class Value : public Node {
	private:
		T value;
	public:
		Value(const T& value)
			:value(value) {}

		virtual std::ostream& write(std::ostream& os) const noexcept override {
			return writeImpl(os, value);
		}
	};

	class Object : public Node {
	private:
		std::unordered_map<std::string, std::shared_ptr<Node>> children;
	public:
		Object() {}

		virtual std::ostream& write(std::ostream& os) const noexcept override {
			os << '{';
			for (auto it = children.begin(); it != children.end(); ++it)
			{
				os << '\"' << it->first << "\":" << *it->second;
				if (std::next(it) != children.end())
					os << ',';
			}
			os << '}';

			return os;
		}


		template<typename T>
		Object& operator()(const std::string& name, const T& value) {
			children[name] = Node::create(value);
			return *this;
		}

		template<typename T>
		Object& operator()(const std::string& name, const std::initializer_list<T>& value) {
			children[name] = Node::create(value);
			return *this;
		}
	};

	template<typename T>
	inline std::shared_ptr<Node> Node::create(const T& rval) {
		return std::make_shared<Value<T>>(rval);
	};

	inline std::shared_ptr<Node> Node::create(const Object& rval) {
		return std::make_shared<Object>(rval);
	};

	inline std::shared_ptr<Node> Node::create(const char* rval) {
		return std::make_shared<Value<std::string>>(rval);
	}

	template<typename T>
	inline std::shared_ptr<Node> Node::create(const std::initializer_list<T>& value) {
		return std::make_shared<Array<T>>(value);
	}

	template<typename T>
	inline std::shared_ptr<Node> Node::create(const std::vector<T>& value) {
		return std::make_shared<Array<T>>(value);
	}
}


/*Example
int main()
{

	Json::Object root;
	root("ahoj", "ca\"wes");
	root("stringProp", "marek")
		("intProp",258)
		("stringArray", { 
			"ahoj",
			"marek",
			"je",
			"v poli" 
		})
		("intArr", {
			1,
			2,
			3,
			4
			})
		("obj", Json::Object()
			("objIntProp", 2)
			("objDoubleProp", 20.5)
			("arrInObj",{20,10})
		)
		("objArray", { Json::Object()
			("objStrProp", "sevas")
			("objDoubleProp", 20.5)
			("arrInObjInArr",{20,10}),
			Json::Object()
			("objStrProp", "sevas2")
			("objDoubleProp", 22.5)
			("arrInObjInArr",{22,12}) 
			});

	std::vector<int> values = {1,2,3,4,5,6,7};
	std::vector<Json::Object> jsonVect;
	for (int val : values)
	{
		int refval = val + 5;
		jsonVect.push_back(Json::Object()
			("val", val)
			("refVal", refval));
	}
	root("refObjArr", jsonVect);
	std::time_t t = std::time(0);   // get time now
	std::tm dt;
	localtime_s(&dt,&t);
	std::cout << root << std::endl << std::endl << Json::Array<Json::Object>({ Json::Object()
			("objStrProp", "sevas")
			("objDoubleProp", 20.5)
			("date",dt)
			("arrInObjInArr",{20,10}) });
}*/

#endif