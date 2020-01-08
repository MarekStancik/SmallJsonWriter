#ifndef JsonWriterH
#define JsonWriterH
#include <vector>
#include <unordered_map>
#include <memory>

namespace Json
{
	class Node {
	public:
		virtual std::ostream& write(std::ostream& os) const noexcept = 0;

		friend std::ostream& operator<<(std::ostream& os, const Node& node) {
			return node.write(os);
		}

		virtual ~Node() {};
	};

	template<typename T>
	class Array : public Node {
	private:
		std::vector<T> children;
	public:
		Array(std::vector<T>&& children)
			:children(std::move(children)) {}

		Array& operator()(T&& val) {
			children.push_back(std::move(val));
			return *this;
		}

		virtual std::ostream& write(std::ostream& os) const noexcept override {
			os << '[';
			for (auto it = children.begin(); it != children.end(); ++it) {
				if (std::is_same<T, std::string>::value || std::is_same<T, const char*>::value)
					os << '\"' << *it << '\"';
				else
					os << *it;
				if (std::next(it) != children.end())
					os << ',';
			}
			os << ']';

			return os;
		}
	};

	template<typename T,
		typename std::enable_if<!std::is_arithmetic<T>::value>::type * = nullptr>
	std::ostream& writeImpl(std::ostream& os, const T& value) {
		return os << '\"' << value << '\"';
	}

	template<typename T,
		typename std::enable_if<std::is_arithmetic<T>::value>::type * = nullptr>
	std::ostream& writeImpl(std::ostream& os, const T& value) {
		return os << value;
	}

	template<typename T>
	class Value : public Node {
	private:
		T value;
	public:
		Value(const T& value)
			:value(value) {}

		virtual std::ostream& write(std::ostream& os) const noexcept override {
			return writeImpl<T>(os, value);
		}
	};

	template<>
	std::ostream& Value<std::tm>::write(std::ostream& os) const noexcept {
		return os << '\"' << std::put_time(&value, "%Y-%m-%dT%H:%M:%S") << '\"';
	}

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


		template<typename T,
			typename std::enable_if<!std::is_same<typename std::remove_reference<T>::type, Object>::value>::type * = nullptr>
		Object& operator()(const std::string& name,const T& value) {
			children[name] = std::make_shared<Value<typename std::remove_reference<T>::type>>(value);
			return *this;
		}

		template<typename T,
			typename std::enable_if<std::is_same<typename std::remove_reference<T>::type, Object>::value>::type * = nullptr>
		Object& operator()(const std::string& name, T&& value) {
			children[name] = std::make_shared<Object>(std::forward<T>(value));
			return *this;
		}

		template<typename T>
		Object& operator()(const std::string& name, std::initializer_list<T>&& value) {
			children[name] = std::make_shared<Array<T>>(std::forward<std::initializer_list<T>>(value));
			return *this;
		}

		template<typename T>
		Object& operator()(const std::string& name, std::vector<T>&& value) {
			children[name] = std::make_shared<Array<T>>(std::forward<std::vector<T>>(value));
			return *this;
		}
	};
}


/*Example
int main()
{
	Object root;
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
		("obj", Object()
			("objIntProp", 2)
			("objDoubleProp", 20.5)
			("arrInObj",{20,10})
		)
		("objArray", { Object()
			("objStrProp", "sevas")
			("objDoubleProp", 20.5)
			("arrInObjInArr",{20,10}),
			Object()
			("objStrProp", "sevas2")
			("objDoubleProp", 22.5)
			("arrInObjInArr",{22,12})
			});
	std::cout << root << std::endl << Array<Object>({ Object()
			("objStrProp", "sevas")
			("objDoubleProp", 20.5)
			("arrInObjInArr",{20,10}) });
}*/

#endif