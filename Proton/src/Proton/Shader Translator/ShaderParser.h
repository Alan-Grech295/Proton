#pragma once
#include <vector>
#include <filesystem>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <functional>

namespace Proton
{
	namespace Parser
	{
		class Node
		{
		public:
			enum class NodeType { None = 0, VarInit, Variable, Function, Struct, Scope, Binary, Constant, FunctionCall, Group, ValueAccess, Cast, Return, /*End of OpNodes*/EOL, Comma, EndScope, StartGroup, EndGroup};
			Node(NodeType type)
				:
				type(type)
			{}

			Node()
				:
				type(NodeType::None)
			{}

			NodeType type;
		};
	}

	struct Token
	{
		friend class ShaderParser;
	public:
		enum class Type {
			Invalid = 0,
			ShaderType, VarType,
			Plus, Minus, Multiplication, Division, Equals,
			LParen, RParen, LCurly, RCurly, Dot, Comma,
			Int, Float, Double,
			GreaterThan, LessThan, EqualTo, NotEqualTo, GreaterThanOrEqual, LessThanOrEqual,
			Struct, Constructor,
			Return, EOL,
			Semantic,
			Include,
			Variable, SelfVar,
			Exclem,


			Last = Exclem
		};

		Token(Type type, void* value, uint32_t line, uint32_t col)
			:
			type(type),
			value(value),
			line(line),
			col(col)
		{
			std::ostringstream oss;
			std::ostringstream rawValOss;
			oss << typeToString[type];
			if (value)
			{
				oss << ": ";
				switch (type)
				{
				case Type::Int:
					rawValOss << *(int*)value;
					break;
				case Type::Float:
					rawValOss << *(float*)value;
					break;
				case Type::Double:
					rawValOss << *(double*)value;
					break;
				default:
					rawValOss << (char*)value;
				}

				oss << rawValOss.str();
			}
			else
			{
				rawValOss << oss.str();
			}

			desc = oss.str();
			raw = rawValOss.str();
		}

		std::string& ToString()
		{
			return desc;
		}

		const std::string& RawString()
		{
			return raw;
		}

		uint32_t GetLine()
		{
			return line;
		}

		uint32_t GetCol()
		{
			return col;
		}

		Type GetType()
		{
			return type;
		}

		bool IsType(Type t)
		{
			return t == type;
		}

	private:
		std::string desc;
		std::string raw;
		Type type;
		void* value;
		uint32_t line;
		uint32_t col;

	private:
		static std::unordered_map<Type, std::string> typeToString;
	};

	class OpNode : public Parser::Node
	{
	public:
		enum class OpType {None = 0, VarInit, Variable, Function, Struct, Scope, Binary, Constant, FunctionCall, Group, ValueAccess, Cast, Return, _End};
		OpNode(OpType type)
			:
			Node((Node::NodeType)type),
			type(type)
		{}
		OpType type;

		virtual const std::string ToString() = 0;
	};

	class Variable
	{
	public:
		Variable(const std::string& varName, const std::string& varType)
			:
			varName(varName),
			varType(varType)
		{}

		std::string ToString()
		{
			std::string output;

			Variable* nextParent = parent;

			if (!parent)
			{
				return varName;
			}

			output = "." + varName;

			while (nextParent)
			{
				output = "." + nextParent->varName + output;
				nextParent = parent->parent;
			}

			return output.substr(1, output.size() - 1);
		}

		void SetRoot(Variable* root)
		{
			if (parent)
				parent->SetRoot(root);
			else
				parent = root;
		}

		//Naive/non-scalable way of checking if var type is vector
		bool IsVector()
		{
			if (varType.substr(0, 3) == "int")
			{
				char vecNum = varType.at(3);
				return vecNum == '2' || vecNum == '3' || vecNum == '4';
			}

			if (varType.substr(0, 5) == "float")
			{
				char vecNum = varType.at(5);
				return vecNum == '2' || vecNum == '3' || vecNum == '4';
			}

			return false;
		}

		//Naive/non-scalable way of checking if var type is scalar
		bool IsScalar()
		{
			return varType == "int" || varType == "float";
		}

		//Naive/non-scalable way of checking if var type is vector
		static bool IsVector(const std::string& type)
		{
			if (type.substr(0, 3) == "int")
			{
				char vecNum = type.at(3);
				return vecNum == '2' || vecNum == '3' || vecNum == '4';
			}

			if (type.substr(0, 5) == "float")
			{
				char vecNum = type.at(5);
				return vecNum == '2' || vecNum == '3' || vecNum == '4';
			}

			return false;
		}

		//Naive/non-scalable way of checking if var type is scalar
		static bool IsScalar(const std::string& type)
		{
			return type == "int" || type == "float";
		}

	public:
		std::string varType;
		std::string varName;
		Variable* parent = nullptr;
	};

	class VariableNode : public OpNode, public Variable
	{
	public:
		VariableNode(const std::string& varName, const std::string& varType)
			:
			Variable(varName, varType),
			OpNode(OpNode::OpType::Variable)
		{}

		VariableNode(Variable* var)
			:
			Variable(*var),
			OpNode(OpNode::OpType::Variable)
		{}
			
		const std::string ToString() override
		{
			std::string output;

			Variable* nextParent = parent;

			if (!parent)
			{
				return varName;
			}

			output = "." + varName;

			while (nextParent)
			{
				output = "." + nextParent->varName + output;
				nextParent = parent->parent;
			}

			return output.substr(1, output.size() - 1);
		}

		void SetVariable(Variable* var)
		{

			this->varType = var->varType;
			this->varName = var->varName;
			this->parent = var->parent;
		}
	};

	class VariableInitNode : public OpNode
	{
	public:
		VariableInitNode(const std::string& varName, const std::string& varType)
			:
			OpNode(OpType::VarInit),
			varName(varName),
			varType(varType)
		{}

		virtual const std::string ToString() override
		{
			return "Variable Init: Name: " + varName + ", Var Type: " + varType;
		}

		Variable GetVariableNode()
		{
			return { varName, varType };
		}

	public:
		std::string varName;
		std::string varType;
	};

	class ConstantNode : public OpNode
	{
	public:
		ConstantNode(const std::string& type, const std::string& value)
			:
			OpNode(OpType::Constant),
			valueType(type),
			value(value)
		{}

		virtual const std::string ToString() override
		{
			return "Consant: Type: " + valueType + ", Value: " + value;
		}

	public:
		std::string valueType;
		std::string value;
	};

	class ScopeNode : public OpNode
	{
	public:
		ScopeNode()
			:
			OpNode(OpType::Scope),
			opNodes(std::vector<OpNode*>())
		{}

		void AddOpNode(OpNode* node)
		{
			opNodes.push_back(node);
		}

		virtual const std::string ToString() override
		{
			return "Scope: Num Nodes: " + opNodes.size();
		}

	public:
		std::vector<OpNode*> opNodes;
	};

	class Scoped
	{
	public:
		Scoped()
			:
			scope(new ScopeNode())
		{}
	public:
		ScopeNode* scope;
	};

	class FunctionNode : public OpNode, public Scoped
	{
	public:
		FunctionNode(const std::string& retType, const std::string& funName)
			:
			OpNode(OpType::Function),
			retType(retType),
			funName(funName)
		{}

		virtual const std::string ToString() override
		{
			return "Function: Name: " + funName + ", Return Type: " + retType + ", Num Params: " + std::to_string(params.size());
		}

	public:
		//Var Name, Var Type
		std::vector<std::pair<std::string, std::string>> params;
		std::string retType;
		std::string funName;
	};

	class FunctionCallNode : public OpNode
	{
	public:
		FunctionCallNode(const std::string& funName, const std::string& retType)
			:
			OpNode(OpType::FunctionCall),
			funName(funName),
			retType(retType)
		{}

		virtual const std::string ToString() override
		{
			return "Function Call: Name: " + funName + ", Return Type: " + retType + ", Num Args: " + std::to_string(args.size());
		}

		void AddArg(OpNode* arg)
		{
			args.push_back(arg);
		}

	private:
		std::vector<OpNode*> args;
		std::string retType;
		std::string funName;
	};

	class StructNode : public OpNode, public Scoped
	{
	public:
		StructNode(const std::string& structName)
			:
			OpNode(OpType::Struct),
			structName(structName)
		{}

		virtual const std::string ToString() override
		{
			return "Struct: Name: " + structName;
		}

		bool HasVar(const std::string& varName)
		{
			return variables.find(varName) != variables.end();
		}

		std::string GetVarType(const std::string& varName)
		{
			return variables[varName];
		}

	public:
		std::unordered_map<std::string, std::string> variables;
		std::string structName;
	};

	class BinOpNode : public OpNode
	{
	public:
		BinOpNode(Token::Type opType)
			:
			OpNode(OpType::Binary),
			opType(opType)
		{}

		virtual const std::string ToString() override
		{
			std::string nodeType;

			switch (opType)
			{
			case Token::Type::Plus:
				nodeType = "Add";
				break;
			case Token::Type::Minus:
				nodeType = "Subtract";
				break;
			case Token::Type::Multiplication:
				nodeType = "Multiply";
				break;
			case Token::Type::Division:
				nodeType = "Divide";
				break;
			case Token::Type::Equals:
				nodeType = "Assign";
				break;
			}

			return nodeType + ": Left: " + left->ToString() + ", Right: " + right->ToString();
		}

		void SetLeftNode(OpNode* _left)
		{
			left = _left;
		}

		void SetRightNode(OpNode* _right)
		{
			right = _right;
		}

		OpNode* GetLeftNode()
		{
			return left;
		}

		OpNode* GetRightNode()
		{
			return right;
		}

	public:
		Token::Type opType;
		OpNode* left;
		OpNode* right;
	}; 

	class ValueAccessNode : public OpNode
	{
	public:
		ValueAccessNode(OpNode* root, Variable* access)
			:
			OpNode(OpType::ValueAccess),
			root(root),
			access(access)
		{}

		virtual const std::string ToString() override
		{
			return "Value Access node: Type: " + access->varType;
		}

		void SetRoot(OpNode* rootNode)
		{
			root = rootNode;
		}

		Variable* GetAccessNode()
		{
			return access;
		}

	private:
		OpNode* root;
		Variable* access;
	};

	class CastNode : public OpNode
	{
	public:
		CastNode(const std::string& type)
			:
			OpNode(OpType::Cast),
			type(type)
		{}

		void SetCastedNode(OpNode* _castedNode)
		{
			castedNode = _castedNode;
		}

		virtual const std::string ToString() override
		{
			return "Cast: " + castedNode->ToString() + ", Type: " + type;
		}

	private:
		std::string type;
		OpNode* castedNode;
	};

	class GroupNode : public OpNode
	{
	public:
		GroupNode(OpNode* root, const std::string& retType)
			:
			OpNode(OpType::Group),
			root(root),
			retType(retType)
		{}

		GroupNode()
			:
			OpNode(OpType::Group),
			root(nullptr),
			retType("")
		{}

		void SetRoot(OpNode* _root, const std::string& _retType)
		{
			root = _root;
			retType = _retType;
		}

		virtual const std::string ToString() override
		{
			return "Group: " + root->ToString() + ", Type: " + retType;
		}

	public:
		OpNode* root;
		std::string retType;
	};

	class ReturnNode : public OpNode
	{
	public:
		ReturnNode(OpNode* retNode)
			:
			OpNode(OpType::Return),
			retNode(retNode)
		{}

		virtual const std::string ToString() override
		{
			return "Return: " + retNode->ToString();
		}

	private:
		OpNode* retNode;
	};

	class Error
	{
	public:
		enum class Type {
			MissingShaderType, MissingStatement,
			UnrecognisedStatement, MistypedInclude,
			InvalidParameter, UnknownVarType, InvalidSelfAccess, UnrecognisedVariable, UnrecognisedAccessValue,
			VarOutOfScope,
			InvalidGroup,
			UnexpectedEnding,
			
			//Used just as a placeholder until a proper error is made
			PlaceholderError
		};
		Error(Type type, uint32_t line, uint32_t col)
			:
			type(type),
			line(line),
			col(col)
		{}

		Error(const Error& e) = default;

		Type type;

		std::string& ToString() { return description; }
	private:
		virtual void CreateDescription() = 0;
	protected:
		std::string description;
		uint32_t line;
		uint32_t col;
	};

	class PlaceholderError : public Error
	{
	public:
		PlaceholderError(uint32_t line, uint32_t col, const std::string& errorTitle = "")
			:
			Error(Error::Type::PlaceholderError, line, col),
			errorTitle(errorTitle)
		{
			CreateDescription();
		}

		virtual void CreateDescription() override
		{
			std::ostringstream oss;
			oss << errorTitle << " [Line: " << line << ", Column: " << col << "]: ";
			description = oss.str();
		}

	private:
		std::string errorTitle;
	};

	class MissingTypeError : public Error
	{
	public:
		MissingTypeError(uint32_t line, uint32_t col)
			:
			Error(Error::Type::MissingShaderType, line, col) 
		{
			CreateDescription();
		}

		virtual void CreateDescription() override
		{
			description = "Shader Type not found!";
		}
	};

	class UnrecognisedStatementError : public Error
	{
	public:
		UnrecognisedStatementError(uint32_t line, uint32_t col, std::string word)
			:
			Error(Error::Type::UnrecognisedStatement, line, col),
			word(word)
		{
			CreateDescription();
		}

		virtual void CreateDescription() override
		{
			std::ostringstream oss;
			oss << "Unrecognised Statement [Line: " << line << ", Column: " << col << "]: " << word;
			description = oss.str();
		}

	private:
		std::string word;
	};

	class MissingStatementError : public Error
	{
	public:
		MissingStatementError(uint32_t line, uint32_t col)
			:
			Error(Error::Type::MissingStatement, line, col)
		{
			CreateDescription();
		}

		virtual void CreateDescription() override
		{
			std::ostringstream oss;
			oss << "Missing Statement [Line: " << line << ", Column: " << col << "]";
			description = oss.str();
		}
	};

	class MistypedIncludeError : public Error
	{
	public:
		MistypedIncludeError(uint32_t line, uint32_t col, std::string word)
			:
			Error(Error::Type::MistypedInclude, line, col),
			word(word) 
		{
			CreateDescription();
		}

		virtual void CreateDescription() override
		{
			std::ostringstream oss;
			oss << "Mistyped Include [Line: " << line << ", Column: " << col << "]: " << word;
			description = oss.str();
		}

	private:
		std::string word;
	};

	class InvalidParameterError : public Error
	{
	public:
		InvalidParameterError(uint32_t line, uint32_t col, const std::string& param)
			:
			Error(Error::Type::InvalidParameter, line, col),
			param(param)
		{
			CreateDescription();
		}

		virtual void CreateDescription() override
		{
			std::ostringstream oss;
			oss << "Invalid Parameter [Line: " << line << ", Column: " << col << "]: " << param;
			description = oss.str();
		}

	private:
		std::string param;
	};

	class UnknownVariableTypeError : public Error
	{
	public:
		UnknownVariableTypeError(uint32_t line, uint32_t col)
			:
			Error(Error::Type::InvalidParameter, line, col)
		{
			CreateDescription();
		}

		virtual void CreateDescription() override
		{
			std::ostringstream oss;
			oss << "Unknown Variable Type [Line: " << line << ", Column: " << col << "]";
			description = oss.str();
		}
	};

	class InvalidStructError : public Error
	{
	public:
		InvalidStructError(uint32_t line, uint32_t col, const char* structName)
			:
			Error(Error::Type::InvalidParameter, line, col),
			structName(structName)
		{
			CreateDescription();
		}

		virtual void CreateDescription() override
		{
			std::ostringstream oss;
			oss << "Invalid Struct [Line: " << line << ", Column: " << col << "]: " << structName;
			description = oss.str();
		}

	private:
		const char* structName;
	};

	class InvalidConstructorError : public Error
	{
	public:
		InvalidConstructorError(uint32_t line, uint32_t col, const char* constructorName)
			:
			Error(Error::Type::InvalidParameter, line, col),
			constructorName(constructorName)
		{
			CreateDescription();
		}

		virtual void CreateDescription() override
		{
			std::ostringstream oss;
			oss << "Invalid Constructor [Line: " << line << ", Column: " << col << "]: " << constructorName;
			description = oss.str();
		}

	private:
		const char* constructorName;
	};

	class InvalidSelfAccessError : public Error
	{
	public:
		InvalidSelfAccessError(uint32_t line, uint32_t col)
			:
			Error(Error::Type::InvalidSelfAccess, line, col)
		{
			CreateDescription();
		}

		virtual void CreateDescription() override
		{
			std::ostringstream oss;
			oss << "Invalid Self Access [Line: " << line << ", Column: " << col << "]";
			description = oss.str();
		}
	};

	class UnrecognisedVariableError : public Error
	{
	public:
		UnrecognisedVariableError(uint32_t line, uint32_t col, const std::string& varName)
			:
			Error(Error::Type::UnrecognisedVariable, line, col),
			varName(varName)
		{
			CreateDescription();
		}

		virtual void CreateDescription() override
		{
			std::ostringstream oss;
			oss << "Unrecognised Variable [Line: " << line << ", Column: " << col << "]: " << varName;
			description = oss.str();
		}
	private:
		std::string varName;
	};

	class UnrecognisedAccessValueError : public Error
	{
	public:
		UnrecognisedAccessValueError(uint32_t line, uint32_t col, const std::string& accessName)
			:
			Error(Error::Type::UnrecognisedAccessValue, line, col),
			accessName(accessName)
		{
			CreateDescription();
		}

		virtual void CreateDescription() override
		{
			std::ostringstream oss;
			oss << "Unrecognised Access Value [Line: " << line << ", Column: " << col << "]: " << accessName;
			description = oss.str();
		}
	private:
		std::string accessName;
	};

	class InvalidGroupError : public Error
	{
	public:
		InvalidGroupError(uint32_t line, uint32_t col)
			:
			Error(Error::Type::InvalidGroup, line, col)
		{
			CreateDescription();
		}

		virtual void CreateDescription() override
		{
			std::ostringstream oss;
			oss << "Invalid Group [Line: " << line << ", Column: " << col << "]";
			description = oss.str();
		}
	};

	class VarOutOfScopeError : public Error
	{
	public:
		VarOutOfScopeError(uint32_t line, uint32_t col, std::string varName)
			:
			Error(Error::Type::VarOutOfScope, line, col),
			varName(varName)
		{
			CreateDescription();
		}

		virtual void CreateDescription() override
		{
			std::ostringstream oss;
			oss << "Variable Out of Scope [Line: " << line << ", Column: " << col << "]: " << varName;
			description = oss.str();
		}
	private:
		std::string varName;
	};

	class UnexpectedEndingError : public Error
	{
	public:
		UnexpectedEndingError(uint32_t line, uint32_t col)
			:
			Error(Error::Type::UnexpectedEnding, line, col)
		{
			CreateDescription();
		}

		virtual void CreateDescription() override
		{
			std::ostringstream oss;
			oss << "Unexpected Ending [Line: " << line << ", Column: " << col << "]";
			description = oss.str();
		}
	};

	struct ShaderProfile : public Scoped
	{
	public:
		ShaderProfile()
		{
		}
	public:
		std::vector<std::pair<std::string, std::string>> globalVars;
	};

	class ShaderParser
	{
	public:
		ShaderParser();

		static ShaderProfile* ParseShader(const std::string& path);

		static std::vector<Token>& LexShader(const std::string& path);

		static std::vector<std::pair<Error*, uint32_t>>& GetErrors() { return Get().lastErrors; }
	private:
		static ShaderParser& Get()
		{
			static ShaderParser parser;
			return parser;
		}

		static std::string GetWord(const std::string& line, int& i);

		static std::string ReadString(const std::string& line, int& i);

		static void AddError(Error* error, uint32_t line)
		{
			Get().lastErrors.push_back({ error, line });
		}

		struct ShaderInfo
		{
			std::string currentStructName;
			std::unordered_map<std::string, StructNode*> structs;
			std::vector<std::pair<std::string, std::pair<std::string, ScopeNode*>>> variables;
			//std::unordered_map<std::string, std::pair<std::string, ScopeNode*>> variables;

			std::pair<std::string, ScopeNode*>* GetVariable(const std::string& varName)
			{
				for (auto& varInfo : variables)
				{
					if (varInfo.first == varName)
						return &varInfo.second;
				}

				return nullptr;
			}

			void AddVariable(const std::string& varName, const std::string& varType, ScopeNode* currentScope)
			{
				variables.push_back({ varName, {varType, currentScope} });
			}

			void PopScopeVars(ScopeNode* scope)
			{
				while (variables[variables.size() - 1].second.second == scope)
					variables.pop_back();
			}

			bool HasStruct(const std::string& structName)
			{
				return structs.find(structName) != structs.end();
			}
		};

		static Parser::Node* GetNextNode(ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i);

		static std::pair<OpNode*, std::string>* ParseEquation(ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i);
		
		static std::tuple<OpNode*, GroupNode*, std::vector<Token>*>* GetGroup(ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i);

		static bool IsSymbol(char c)
		{
			return Get().symbols.find(c) != Get().symbols.end();
		}

		static bool IsVarType(const std::string& str)
		{
			return Get().varTypes.find(str) != Get().varTypes.end();
		}

		static bool IsStruct(const std::string& str)
		{
			return Get().structs.find(str) != Get().structs.end();
		}

		static bool ValidVariableName(const std::string& name)
		{
			if (Get().digits.find(name[0]) != Get().digits.end())
				return false;

			for (char c : name)
			{
				if (c == '.' || c == '-')
					return false;
			}

			return true;
		}

		static std::pair<OpNode*, std::string>* FindTerm(ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i);

		static bool IsNumber(const std::string& word)
		{
			for (char c : word)
			{
				if (Get().digits.find(c) == Get().digits.end() && c != '.' && c != 'f')
					return false;
			}
			return true;
		}

		static Token::Type GetNumberType(const std::string& number)
		{
			if (number[number.length() - 1] == 'f')
				return Token::Type::Float;

			uint32_t dotCount = 0;
			for (char c : number)
			{
				if (c == '.')
					dotCount++;
			}

			if(dotCount > 1)
				return Token::Type::Invalid;

			if(dotCount == 1)
				return Token::Type::Double;

			return Token::Type::Int;
		}

		//Only works for vectors (ex: .xyz, .xyzw)
		static std::string GetAccessValueType(const std::string& accessName)
		{
			//Only handling default vectors (ex: float2, float4)
			if (accessName.length() == 0)
				return "";

			if (accessName.length() > 4)
				return "";

			for (char c : accessName)
			{
				if (c != 'x' && c != 'y' &&
					c != 'z' && c != 'w')
					return "";
			}

			return "vec" + std::to_string(accessName.length());
		}

		//Only works for vectors (ex: .xyz, .xyzw)
		static std::string ValidateVecAccessType(const std::string& accessType, const std::string& accessValue)
		{
			if (accessValue.length() > 4)
				return "";

			std::string accessValueType;
			uint32_t accessVecLength;

			//Only handles float and int vectors for now
			if (accessType.substr(0, 3) == "int")
			{
				accessValueType = "int";
				accessVecLength = std::stoi(accessType.substr(3, 1));
			}
			else if (accessType.substr(0, 5) == "float")
			{
				accessValueType = "float";
				accessVecLength = std::stoi(accessType.substr(5, 1));
			}
			else
			{
				//TODO: Add other types
				return "";
			}

			//Give the respective index for each vector element (ex: x - 1, y - 2, z - 3, w - 4)
			static std::unordered_map<char, uint32_t> vecAccessMap = { {'x', 1}, {'y', 2}, {'z', 3}, {'w', 4} };

			for (char c : accessValue)
			{
				if (c != 'x' && c != 'y' &&
					c != 'z' && c != 'w')
					return "";

				if (vecAccessMap[c] > accessVecLength)
					return "";
			}

			return accessValueType + std::to_string(accessValue.length());
		}
		
		using Node = Parser::Node;
		static void AddNodeFunction(const std::vector<Token::Type>& definingTokens, std::function<Node*(ShaderInfo&, std::vector<Token>&, uint32_t&)> func)
		{
			std::string key;
			for (Token::Type type : definingTokens)
			{
				uint32_t typeVal = (uint32_t)type;

				if constexpr (typeByteAmnt == 1)
				{
					key += *(char*)(&typeVal);
				}
				else
				{
					for (int i = 0; i < typeByteAmnt; i++)
						key += ((char*)(&typeVal))[i];
				}
			}

			if (definingTokens.size() > maxDefiningTokens)
				maxDefiningTokens = definingTokens.size();

			nodeFunctions[key] = func;
		}

		static bool SetAccessValueTypes(ShaderInfo& info, Variable* access, std::string accessedType);

#define CHECK_NODE(node) if(!node) return nullptr;
#define ADD_ERROR(error) AddError(error, __LINE__)
#define CHECK_END if(i >= tokens.size()) { ADD_ERROR(new UnexpectedEndingError(tokens[tokens.size() - 1].line, tokens[tokens.size() - 1].col)); return nullptr; }
#define CHECK_ERRORS if (Get().lastErrors.size() > 0) return nullptr;

	private:
		std::vector<std::pair<Error*, uint32_t>> lastErrors;
		std::vector<Token> tokens;

		std::unordered_set<std::string> varTypes;
		std::unordered_set<std::string> structs;
		std::unordered_map<char, Token::Type> symbols;

		std::unordered_set<char> digits;

		static std::unordered_map<std::string, std::function<Parser::Node*(ShaderInfo&, std::vector<Token>&, uint32_t&)>> nodeFunctions;

		std::string currentStruct;
		bool insideConstructor;	

		//If the total amount of token types is larger than 255, set the amount of storing 
		//characters to 2 (should never need to be higher)
		static constexpr uint32_t typeByteAmnt = ((uint32_t)Token::Type::Last > 255) ? 2 : 1;
		static uint32_t maxDefiningTokens;
	};
}