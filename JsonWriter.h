#ifndef JsonWriterH
#define JsonWriterH
#include <vector>
#include <unordered_map>
#include <memory>

namespace Json
{
	namespace details 
	{
		template <class charT, charT sep>
		class punct_facet : public std::numpunct<charT> {
		protected:
			charT do_decimal_point() const { return sep; }
		};

		typedef punct_facet<char, '.'> DecimalPointFacet;
	}

	class Node;
	template<typename T>
	class Value;
	class Object;
	template<typename T>
	class Array;

	class Node {
	private:
		virtual std::ostream& write(std::ostream& os) const noexcept = 0;
	protected:
		//------------WriterImpl---------------//
		template<typename T>
		inline static std::ostream& writeImpl(std::ostream& os, const T& value) noexcept {
			return os << value;
		}

		inline static std::ostream& writeImpl(std::ostream& os, const std::tm& value) {
			return os << '\"' << std::put_time(&value, "%Y-%m-%dT%H:%M:%S") << '\"';
		}

		static std::ostream& writeImpl(std::ostream& os, const std::string& value) noexcept{
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
	public:
		friend std::ostream& operator<<(std::ostream& os, const Node& node) {
			os.imbue(std::locale(os.getloc(),new details::DecimalPointFacet()));
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


	template<typename T>
	class Array : public Node {
	private:
		std::vector<T> children;

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
	public:
		Array(const std::vector<T>& children)
			:children(children) {}

		Array& operator()(T&& val) {
			children.push_back(std::move(val));
			return *this;
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
	public:
		Object() {}

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