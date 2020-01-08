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
		for (auto it = children.begin(); it != children.end(); ++it){
			if (std::is_same<T,std::string>::value || std::is_same<T,const char*>::value)
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

template<typename T>
class ValueNode : public Node {
private:
	T value;
public:
	ValueNode(T&& value)
		:
		value(std::move(value))
	{

	}

	virtual std::ostream& write(std::ostream& os) const noexcept override {
		if (!std::is_arithmetic<T>::value)
			os << '\"' << value << '\"';
		else
			os << value;

		return os;
	}
};

class Object : public Node {
private:
	std::unordered_map<std::string,std::shared_ptr<Node>> children;
public:
	Object(){}

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
	Object& operator()(const std::string& name, T && value) {
		children[name] = std::make_shared<ValueNode<T>>(std::move(value));
		return *this;
	}

	template<typename T>
	Object& operator()(const std::string& name, std::initializer_list<T>&& value) {
		children[name] = std::make_shared<Array<T>>(std::forward<std::initializer_list<T>>(value));
		return *this;
	}
};

template<>
Object& Object::operator()(const std::string& name, Object& value) {
	children[name] = std::make_shared<Object>(value);
	return *this;
}

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