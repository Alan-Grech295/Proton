#include "ptpch.h"
#include "ShaderParser.h"
#include <fstream>
#include "Proton\Core\Log.h"

namespace Proton
{
	std::unordered_map<std::string, std::function<Parser::Node*(ShaderParser::ShaderInfo&, std::vector<Token>&, uint32_t&)>> ShaderParser::nodeFunctions;
	uint32_t ShaderParser::maxDefiningTokens;

	std::unordered_map<Token::Type, std::string> Token::typeToString = {
			{Type::Invalid, "Invalid"},
			{Type::VarType, "VarType"},
			{Type::Plus, "Plus"},
			{Type::Minus, "Minus"},
			{Type::Multiplication, "Multiplication"},
			{Type::Division, "Division"},
			{Type::Equals, "Equals"},
			{Type::LParen, "LParen"},
			{Type::RParen, "RParen"},
			{Type::LCurly, "LCurly"},
			{Type::RCurly, "RCurly"},
			{Type::Dot, "Dot"},
			{Type::Int, "Int"},
			{Type::Float, "Float"},
			{Type::Double, "Double"},
			{Type::Struct, "Struct"},
			{Type::Constructor, "Constructor"},
			{Type::Return, "Return"},
			{Type::Semantic, "Semantic"},
			{Type::Variable, "Variable"},
			{Type::SelfVar, "SelfVar"},
			{Type::Include, "Include"},
			{Type::EOL, "End of Line"},
			{Type::ShaderType, "Shader Type"},
			{Type::GreaterThan, "Greater than"},
			{Type::LessThan, "Less than"},
			{Type::GreaterThanOrEqual, "Greater than or equal"},
			{Type::LessThanOrEqual, "Less than or equal"},
			{Type::EqualTo, "Equal to"},
			{Type::NotEqualTo, "Not equal to"},
			{Type::Comma, "Comma"}
	};

	using Node = Parser::Node;

	ShaderParser::ShaderParser()
	{
		varTypes.insert("int");
		varTypes.insert("float");
		varTypes.insert("bool");
		varTypes.insert("matrix");
		varTypes.insert("float2");
		varTypes.insert("float3");
		varTypes.insert("float4");
		varTypes.insert("float3x3");
		varTypes.insert("Texture2D");

		symbols['+'] = Token::Type::Plus;
		symbols['-'] = Token::Type::Minus;
		symbols['*'] = Token::Type::Multiplication;
		symbols['/'] = Token::Type::Division;
		symbols['='] = Token::Type::Equals;
		symbols['('] = Token::Type::LParen;
		symbols[')'] = Token::Type::RParen;
		symbols['{'] = Token::Type::LCurly;
		symbols['}'] = Token::Type::RCurly;
		symbols['.'] = Token::Type::Dot;
		symbols[','] = Token::Type::Comma;
		symbols[';'] = Token::Type::EOL;

		symbols['>'] = Token::Type::GreaterThan;
		symbols['<'] = Token::Type::LessThan;

		symbols['!'] = Token::Type::Exclem;

		digits.insert('0');
		digits.insert('1');
		digits.insert('2');
		digits.insert('3');
		digits.insert('4');
		digits.insert('5');
		digits.insert('6');
		digits.insert('7');
		digits.insert('8');
		digits.insert('9');

		//Node Functions
		using OpType = Node::NodeType;
		using Node = Parser::Node;
		
		AddNodeFunction({ Token::Type::EOL}, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) { return new Node(OpType::EOL); });
		AddNodeFunction({ Token::Type::Comma }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) { return new Node(OpType::Comma); });
		AddNodeFunction({ Token::Type::Return }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) { return new ReturnNode(ParseEquation(info, tokens, i)->first); });
		AddNodeFunction({ Token::Type::LCurly }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) { return new ScopeNode(); });
		AddNodeFunction({ Token::Type::RCurly }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) { return new Node(OpType::EndScope); });

		AddNodeFunction({ Token::Type::RParen }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) -> Node* {
			return new Node(OpType::EndGroup);
		});
		AddNodeFunction({ Token::Type::Dot, Token::Type::Variable }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) -> Node* {
			Token& valAccTok = tokens[i - 1];
			
			/*std::string valueType = GetAccessValueType((char*)valAccTok.value);

			//TODO: Check value type validity
			if (valueType == "")
			{
				ADD_ERROR(new UnrecognisedAccessValueError(valAccTok.line, valAccTok.col, valAccTok.RawString()));
				return nullptr;
			}*/

			Variable* access = new Variable((char*)valAccTok.value, "");
			
			uint32_t pastI = i;
			Node* currentNode = GetNextNode(info, tokens, i);

			if (currentNode && currentNode->type == Node::NodeType::ValueAccess)
			{
				ValueAccessNode* valAccNode = static_cast<ValueAccessNode*>(currentNode);
				Variable* newAccess = valAccNode->GetAccessNode();
				newAccess->SetRoot(access);
				access = newAccess;
			}
			else
			{
				i = pastI;
			}

			/*while (tokens[i].IsType(Token::Type::Dot))
			{
				i++;
				CHECK_END
				if (tokens[i].IsType(Token::Type::Variable))
				{
					valueType = GetAccessValueType((char*)tokens[i].value);

					if (valueType == "")
					{
						ADD_ERROR(new UnrecognisedAccessValueError(tokens[i].line, tokens[i].col, tokens[i].RawString()));
						return nullptr;
					}

					Variable* newAccess = new Variable((char*)tokens[i].value, valueType);
					newAccess->parent = access;
					access = newAccess;
				}
				else
				{
					ADD_ERROR(new UnrecognisedStatementError(tokens[i].line, tokens[i].col, tokens[i].RawString()));
					return nullptr;
				}
				i++;
				CHECK_END
			}*/

			return new ValueAccessNode(nullptr, access);
		});

		AddNodeFunction({ Token::Type::Struct }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) { return new StructNode((char*)tokens[i - 1].value); });

		//Constant Nodes
		AddNodeFunction({ Token::Type::Float }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) { return new ConstantNode("float", std::to_string(*(float*)tokens[i - 1].value)); });
		AddNodeFunction({ Token::Type::Int }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) { return new ConstantNode("int", std::to_string(*(int*)tokens[i - 1].value)); });

		//Operator Nodes
		AddNodeFunction({ Token::Type::Equals }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) { return new BinOpNode(tokens[i - 1].GetType()); });
		AddNodeFunction({ Token::Type::Plus }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) { return new BinOpNode(tokens[i - 1].GetType()); });
		AddNodeFunction({ Token::Type::Minus }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) { return new BinOpNode(tokens[i - 1].GetType()); });
		AddNodeFunction({ Token::Type::Multiplication }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) { return new BinOpNode(tokens[i - 1].GetType()); });
		AddNodeFunction({ Token::Type::Division }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) { return new BinOpNode(tokens[i - 1].GetType()); });
	
		AddNodeFunction({ Token::Type::VarType, Token::Type::Variable }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) -> Node* {
				std::string type = (char*)tokens[i - 2].value;
				std::string varName = std::string((char*)tokens[i - 1].value);

				if (Get().varTypes.find(type) == Get().varTypes.end() && info.structs.find(type) == info.structs.end())
				{
					ADD_ERROR(new UnknownVariableTypeError(tokens[i - 2].line, tokens[i - 2].col));
					return nullptr;
				}

				return new VariableInitNode(varName, type);
			});
		AddNodeFunction({ Token::Type::VarType, Token::Type::Variable, Token::Type::LParen }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) -> Node* {
			std::string funcType = (char*)tokens[i - 3].value;
			std::string funcName = std::string((char*)tokens[i - 2].value);

			if (Get().varTypes.find(funcType) == Get().varTypes.end() && info.structs.find(funcType) == info.structs.end())
			{
				ADD_ERROR(new UnknownVariableTypeError(tokens[i - 2].line, tokens[i - 2].col));
				return nullptr;
			}

			FunctionNode* funcNode = new FunctionNode(funcType, funcName);
			Node* node = GetNextNode(info, tokens, i);
			CHECK_NODE(node);
			bool pastWasArg = node->type == Node::NodeType::VarInit;

			while (node->type != Node::NodeType::EndGroup)
			{
				if (node->type == Node::NodeType::VarInit)
				{
					VariableInitNode* var = static_cast<VariableInitNode*>(node);
					funcNode->params.push_back({ var->varName, var->varType });
				}
				else if (!(node->type == Node::NodeType::Comma && pastWasArg))
				{
					ADD_ERROR(new InvalidParameterError(tokens[i].line, tokens[i].col, tokens[i].RawString()));
					return nullptr;
				}

				pastWasArg = node->type == Node::NodeType::VarInit;

				node = GetNextNode(info, tokens, i);
				CHECK_NODE(node);
			}

			return funcNode;
			
			});
	
		AddNodeFunction({ Token::Type::Constructor, Token::Type::LParen }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) -> Node* {
			FunctionNode* funcNode = new FunctionNode((char*)tokens[i - 2].value, "#constructor");
			Node* node = GetNextNode(info, tokens, i);
			CHECK_NODE(node);
			bool pastWasArg = node->type == Node::NodeType::VarInit;

			while (node->type != Node::NodeType::EndGroup)
			{
				if (node->type == Node::NodeType::VarInit)
				{
					VariableInitNode* var = static_cast<VariableInitNode*>(node);
					funcNode->params.push_back({ var->varName, var->varType });
				}
				else if (!(node->type == Node::NodeType::Comma && pastWasArg))
				{
					ADD_ERROR(new InvalidParameterError(tokens[i].line, tokens[i].col, tokens[i].RawString()));
					return nullptr;
				}

				pastWasArg = node->type == Node::NodeType::VarInit;

				node = GetNextNode(info, tokens, i);
				CHECK_NODE(node);
			}

			//TODO: Check for invalid constructors
			return funcNode;
			});

		AddNodeFunction({ Token::Type::LParen }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) -> Node* { return new Node(OpType::StartGroup); });
		AddNodeFunction({ Token::Type::LParen, Token::Type::VarType, Token::Type::RParen }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) -> Node* { return new CastNode((char*)tokens[i - 2].value); });
	
		AddNodeFunction({ Token::Type::VarType, Token::Type::LParen }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) -> Node* {
			FunctionCallNode* functionCallNode = new FunctionCallNode("#constructor", (char*)tokens[i - 2].value);
			//TODO: Parameter type check
			if (tokens[i].IsType(Token::Type::RParen))
			{
				i++;
				return functionCallNode;
			}

			OpNode* arg = nullptr;

			do
			{
				//TODO: Use return type to check argument type validity
				arg = ParseEquation(info, tokens, i)->first;

				if (!tokens[i - 1].IsType(Token::Type::RParen) && tokens[i].IsType(Token::Type::EOL))
				{
					PT_CORE_ERROR("End of function call not found!");
					break;
				}

				if (!arg)
				{
					ADD_ERROR(new UnrecognisedStatementError(tokens[i].line, tokens[i].col, (char*)tokens[i].value));
					return nullptr;
				}

				//TODO: Check if argument is valid
				functionCallNode->AddArg(arg);
			} while (!tokens[i - 1].IsType(Token::Type::RParen));

			return functionCallNode;
			});
	
		AddNodeFunction({ Token::Type::Variable }, [](ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i) -> Node* {
			Token& varTok = tokens[i - 1];

			bool self = std::string((char*)varTok.value) == "this" && info.currentStructName != "";

			if (!self && !info.GetVariable((char*)varTok.value))
			{
				ADD_ERROR(new UnrecognisedVariableError(varTok.line, varTok.col, (char*)varTok.value));
				return nullptr;
			}

			std::string varStructName;
			VariableNode* var;

			if (self)
			{
				varStructName = info.currentStructName;
				var = new VariableNode((char*)varTok.value, info.currentStructName);
			}
			else
			{
				std::pair<std::string, ScopeNode*>& varInfo = *info.GetVariable((char*)varTok.value);
				varStructName = varInfo.first;

				var = new VariableNode((char*)varTok.value, varInfo.first);
			}

			if (info.structs.find(varStructName) == info.structs.end())
			{
				if (tokens[i].IsType(Token::Type::Dot))
				{
					i++;
					PT_CORE_WARN("Non struct variable access not handled yet!");
				}

				return var;
			}

			uint32_t pastI = i;
			Node* nextNode = GetNextNode(info, tokens, i);

			if (nextNode->type == Node::NodeType::ValueAccess)
			{
				ValueAccessNode* valAccNode = static_cast<ValueAccessNode*>(nextNode);
				if (!SetAccessValueTypes(info, valAccNode->GetAccessNode(), var->varType))
				{
					//TODO: Add respective error
					ADD_ERROR(new PlaceholderError(tokens[i].line, tokens[i].col, tokens[i].ToString()));
					return nullptr;
				}

				valAccNode->GetAccessNode()->SetRoot(var);
				var = new VariableNode(valAccNode->GetAccessNode());
			}
			else
			{
				i = pastI;
			}

			/*while (tokens[i].IsType(Token::Type::Dot))
			{
				i++;
				CHECK_END

				Token& varTok = tokens[i++];
				CHECK_END
				char* varName = (char*)varTok.value;

				//TODO: Handle non struct variable children (Ex: vec.x)
				StructNode* struc = info.structs[var->varType];
				if (struc->HasVar(varName))
				{
					VariableNode* childVar = new VariableNode(varName, struc->GetVarType(varName));

					childVar->parent = var;
					var = childVar;

					if (info.structs.find(childVar->varType) == info.structs.end())
					{
						if (tokens[i].IsType(Token::Type::Dot))
						{
							i++;
							PT_CORE_WARN("Non struct variable access not handled yet!");
						}

						return var;
					}
				}
				else
				{
					ADD_ERROR(new UnrecognisedVariableError(varTok.line, varTok.col, (char*)varTok.value));
					return nullptr;
				}
			}*/

			return var;

			});
	}		

	static bool IsComment(int& i, const std::string& line, bool& multiLineComment)
	{
		char c = line[i];

		if (c == '/' && i < line.length() - 1)
		{
			if (line[i + 1] == '/')
				return true;

			if (line[i + 1] == '*')
			{
				multiLineComment = true;
				i++;
			}
		}

		return false;
	}

	static bool EndMultiline(int& i, const std::string& line)
	{
		char c = line[i];
		if (c == '*' && i < line.length() - 1 && line[i + 1] == '/')
		{
			i++;
			return true;
		}

		return false;
	}

	static void* StrToVoidPtr(const std::string& str)
	{
		char* charPtr = new char[str.length() + 1];
		strcpy(charPtr, str.c_str());

		return (void*)charPtr;
	}

	ShaderProfile* ShaderParser::ParseShader(const std::string& path)
	{
		using Type = Token::Type;
		using NodeType = Parser::Node::NodeType;
		
		std::vector<Token>& tokens = LexShader(path);
		ShaderProfile* profile = new ShaderProfile();
		uint32_t scopeLevel = 0;
		std::unordered_map<std::string, StructNode*> structs;

		CHECK_ERRORS

		//A list of scopes 
		std::vector<ScopeNode*> scopes;
		ScopeNode* currentScope = profile->scope;
		scopes.push_back(profile->scope);

		uint32_t i = 1;
		ShaderInfo info;

		Node* pastNode = new Node(NodeType::None);
		Node* savedNode = new Node(NodeType::None);
		OpNode* currentOpNode = nullptr;
		NodeType nextNeededType = NodeType::None;

		//Decides whether the current node should be pushed to the scope
		bool setCurrentOpNode = false;

		while (i < tokens.size())
		{
			Node* node = GetNextNode(info, tokens, i);
			CHECK_ERRORS

			setCurrentOpNode = false;

			if (!node || node->type == NodeType::None)
				continue;

			switch (node->type)
			{
			case NodeType::Struct:
				//Sets the current struct to the received struct node
				info.currentStructName = static_cast<StructNode*>(node)->structName;
				info.structs[info.currentStructName] = static_cast<StructNode*>(node);

				//As a struct needs a scope, the next needed node is a scope node,
				//otherwise there is an error
				nextNeededType = NodeType::Scope;
				setCurrentOpNode = true;
				//Saves the struct node in order to then apply the scope node when it is recieved
				savedNode = node;
				break;
			case NodeType::Function:
				//As a function needs a scope, the next needed node is a scope node,
				//otherwise there is an error
				nextNeededType = NodeType::Scope;
				savedNode = node;
				//Saves the functions node in order to then apply the scope node when it is recieved
				setCurrentOpNode = true;
				break;
			case NodeType::Variable:
			{
				//TODO: Add variable functionality and use
				VariableNode* var = static_cast<VariableNode*>(node);
			}
				break;
			case NodeType::Scope:
				//Add the current scope to the list of scopes
				scopes.push_back(static_cast<ScopeNode*>(node));

				//If the next needed type is a scope, set the scope node of the respective node to this scope node
				if (nextNeededType == NodeType::Scope)
				{
					switch (savedNode->type)
					{
					case NodeType::Function:
					{
						FunctionNode* funcNode = static_cast<FunctionNode*>(savedNode);
						ScopeNode* curScope = scopes[scopes.size() - 1];
						funcNode->scope = curScope;

						//Adds the function parameters to the available scope variables
						for (auto& param : funcNode->params)
						{
							info.AddVariable(param.first, param.second, curScope);
						}
					}
						break;
					case NodeType::Struct:
						static_cast<StructNode*>(savedNode)->scope = scopes[scopes.size() - 1];
						break;
					}
					savedNode->type = NodeType::None;
				}
				else
				{
					//Adds this scope node to the current scope node
					currentScope->AddOpNode(static_cast<ScopeNode*>(node));
				}

				//If there is a set OpNode then add it to this scope
				if (currentOpNode)
				{
					currentScope->AddOpNode(currentOpNode);
					currentOpNode = nullptr;
				}

				//Sets the current scope to the last scope in the list (this node)
				currentScope = scopes[scopes.size() - 1];

				nextNeededType = NodeType::None;
				break;
			case NodeType::VarInit:
			{
				VariableInitNode* var = static_cast<VariableInitNode*>(node);
				//Adds the current variable to the shader info
				info.AddVariable(var->varName, var->varType, currentScope);
				setCurrentOpNode = true;

				//If this variable initialization is inside a struct, add this variable to the struct variables
				if (info.currentStructName != "" && currentScope == info.structs[info.currentStructName]->scope)
					info.structs[info.currentStructName]->variables[var->varName] = var->varType;
			}
				break;
			case NodeType::EndScope:
				//When a scope is closed then remove all the scope's variables
				info.PopScopeVars(currentScope);
				//Remove the scope from the list
				scopes.pop_back();
				//Set the current scope to the last scope
				currentScope = scopes[scopes.size() - 1];
				break;
			case NodeType::EndGroup:
				
				break;
			case NodeType::EOL:
				//Since it is the end of line, if there is a current OpNode
				//add it to the current scope as no further processing can be
				//done on this node
				if (currentOpNode)
				{
					currentScope->AddOpNode(currentOpNode);
					currentOpNode = nullptr;
				}
				else
				{
					ADD_ERROR(new PlaceholderError(tokens[i].line, tokens[i].col, "No Node to Add"));
				}

				//Clear any needed nodes
				nextNeededType = NodeType::None;
				break;

				//Binary Operations
			case NodeType::Binary:
			{
				BinOpNode* binOpNode = static_cast<BinOpNode*>(node);
				setCurrentOpNode = true;
				switch (binOpNode->opType)
				{
				case Token::Type::Equals:
					//TODO: Add invalid lhs error
					//Set the left hand of the node to the past node
					binOpNode->SetLeftNode(static_cast<OpNode*>(pastNode));

					//Parse the right hand side of the node
					std::pair<OpNode*, std::string>* rhs = ParseEquation(info, tokens, i);

					if (rhs)
						binOpNode->SetRightNode(rhs->first);
					else
						ADD_ERROR(new MissingStatementError(tokens[i].line, tokens[i].col));
					break;
				}
			}
				break;
			case NodeType::Return:
				setCurrentOpNode = true;
				break;
			}

			if (setCurrentOpNode)
			{
				//Sets the current node to the current op node
				currentOpNode = static_cast<OpNode*>(node);
			}
			else
			{
				//If a next node is needed and the current node is not an OpNode (hence (uint32_t)node->type < (uint32_t)OpNode::OpType::_End) 
				//then there is an error
				if (nextNeededType != NodeType::None && (uint32_t)node->type < (uint32_t)OpNode::OpType::_End && node->type != nextNeededType)
					ADD_ERROR(new UnrecognisedStatementError(tokens[i - 1].line, tokens[i - 1].col, tokens[i - 1].ToString()));
			}

			pastNode = node;

			CHECK_ERRORS
		}

		return profile;
	}

	Node* ShaderParser::GetNextNode(ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i)
	{
		using OpType = Node::NodeType;
		using TokType = Token::Type;

		//Stores the last key that was in the dictionary
		std::string lastWorkingKey;
		//The index of the token of last working key
		uint32_t lastWorkingI = i;
		//The current key
		std::string key;

		//This loop goes over the tokens at a certain point, converts them to characters,
		//appends them to "key" and checks if that key exists. If the key exists it is stored
		//in "lastWorkingKey"
		for (int j = 0; j < maxDefiningTokens && i < tokens.size(); j++)
		{
			//Converts the type to an int in order to then convert it to a char
			uint32_t typeVal = (uint32_t)tokens[i].type;

			//1 char = 1 byte, so if there are more than 255 types the type has to be
			//stored in 2 or more chars. This statement is run at compile time to check
			//if 1 or more characters are to be used.
			if constexpr (typeByteAmnt == 1)
			{
				key += *(char*)(&typeVal);
			}
			else
			{
				for (int i = 0; i < typeByteAmnt; i++)
					key += *(char*)((&typeVal) + i);
			}

			//If the current key exists then save it
			if (Get().nodeFunctions.find(key) != Get().nodeFunctions.end())
			{
				lastWorkingKey = key;
				lastWorkingI = i;
			}

			i++;
		}

		//Set the current i to the last working key index + 1
		i = lastWorkingI + 1;

		//If the key is not empty then run the appropriate function
		if (lastWorkingKey != "")
			return Get().nodeFunctions[lastWorkingKey](info, tokens, i);

		return nullptr;
	}

	std::pair<OpNode*, std::string>* ShaderParser::ParseEquation(ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i)
	{
		/*Equation hierarchy: factor -> term -> equation
		Term structure: (factor 1 *|/ factor 2) factor 2 can be non-existent and there can be multiple factors
		Equation structure: (term 1 +|- term 2) term 2 can be non-existent and there can be multiple terms
		
		Equations enclosed in brackets are taken as factors*/

		using NodeType = Node::NodeType;
		using TokType = Token::Type;

		//Node to return
		OpNode* retNode = nullptr;
		BinOpNode* currentOperationalNode = nullptr;
		//Gets the term
		std::pair<OpNode*, std::string>* currentTermReturned = FindTerm(info, tokens, i);
		OpNode* currentTerm = currentTermReturned->first;
		std::string termValType = currentTermReturned->second;
		CHECK_NODE(currentTerm);

		if (i >= tokens.size()) return new std::pair<OpNode*, std::string>(currentTerm, termValType);

		//Right now the type check just checks if the past type is equal to the current type or 
		//a scalar (for vectors). Therefore different vector types cannot be multiplied (if they
		//even can be in the first place)
		std::string pastType;

		uint32_t preOpI = i;
		Node* currentNode = GetNextNode(info, tokens, i);
		CHECK_NODE(currentNode);

		while (tokens.size() >= i && currentNode->type != Node::NodeType::Comma && currentNode->type != NodeType::EOL && currentNode->type != Parser::Node::NodeType::ValueAccess && currentNode->type != Parser::Node::NodeType::EndGroup)
		{
			//If the past type is empty, assume that this is the first term so set the
			//past type to this term's type
			if (pastType.empty())
			{
				pastType = termValType;
			}
			else
			{
				//If the current term's type is not equal to the past type
				if (termValType != pastType)
				{
					//If the past type is scalar and the current term type is vector,
					//then set the past type to the current term's type as scalar values
					//can be added/subtracted/multiplied/divided with vectors
					if (Variable::IsScalar(pastType) && Variable::IsVector(termValType))
					{
						pastType = termValType;
					}
					//If the current term isn't a scalar, then there is an error
					else if (!Variable::IsScalar(termValType))
					{
						ADD_ERROR(new PlaceholderError(tokens[i].line, tokens[i].col, "Unmatching Type"));
					}
				}
			}

			//The current node should be a binary operation node 
			BinOpNode* currentOp = static_cast<BinOpNode*>(currentNode);

			//If there is a current BinOpNode, then set the rhs to the current term
			//and set the current BinOpNode (currentOp) lhs to the currentOperationalNode
			if (currentOperationalNode)
			{
				currentOperationalNode->right = currentTerm;
				currentOp->left = currentOperationalNode;
			}
			else
			{
				currentOp->left = currentTerm;
			}

			//Set the current operational node to the current BinOpNode
			currentOperationalNode = currentOp;

			//Find the next term
			currentTermReturned = FindTerm(info, tokens, i);
			currentTerm = currentTermReturned->first;
			termValType = currentTermReturned->second;

			CHECK_NODE(currentTerm);
			if (i >= tokens.size()) break;

			preOpI = i;

			//Find the next node
			currentNode = GetNextNode(info, tokens, i);
			CHECK_NODE(currentNode);
		}

		//The current term's type has to be checked as the loop would have
		//ended so the type would not be checked
		if (pastType.empty())
		{
			pastType = termValType;
		}
		else
		{
			//If the current term's type is not equal to the past type
			if (termValType != pastType)
			{
				//If the past type is scalar and the current term type is vector,
				//then set the past type to the current term's type as scalar values
				//can be added/subtracted/multiplied/divided with vectors
				if (Variable::IsScalar(pastType) && Variable::IsVector(termValType))
				{
					pastType = termValType;
				}
				//If the current term isn't a scalar, then there is an error
				else if (!Variable::IsScalar(termValType))
				{
					ADD_ERROR(new PlaceholderError(tokens[i].line, tokens[i].col, "Unmatching Type"));
				}
			}
		}

		//If a current operational node exists then set the current term to its rhs and return it
		if (currentOperationalNode)
		{
			currentOperationalNode->right = currentTerm;
			retNode = currentOperationalNode;
		}
		//If no operational node exists then the equation is made up of only one term, so return it
		else
		{
			retNode = currentTerm;
		}

		//If the current node is a ValueAccess (such as (vec1 + vec2).xyz), then set the
		//index to before it so that it can be processed in another function and not skipped
		if (currentNode->type == Parser::Node::NodeType::ValueAccess)
			i = preOpI;

		//So far, if the currentNode is an end of line, go back one token so that it can be processed in another
		//function and not skipped
		if(currentNode->type != Node::NodeType::Comma && currentNode->type != Parser::Node::NodeType::EndGroup && currentNode->type != Parser::Node::NodeType::ValueAccess)
			i--;

		return new std::pair<OpNode*, std::string>(retNode, pastType);
	}

	std::pair<OpNode*, std::string>* ShaderParser::FindTerm(ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i)
	{
		//TODO: Error checks
		Token& t = tokens[i];

		OpNode* factorNode = nullptr;
		BinOpNode* currentOperatorNode = nullptr;
		uint32_t pastI = i;
		Node* currentNode = GetNextNode(info, tokens, i);
		CHECK_NODE(currentNode);
		CastNode* currentCast = nullptr;

		//Right now the type check just checks if the past type is equal to the current type or 
		//a scalar (for vectors). Therefore different vector types cannot be multiplied (if they
		//even can be in the first place)
		std::string pastType;
		
		while (tokens.size() >= i && currentNode->type != Node::NodeType::Comma && currentNode->type != Node::NodeType::EOL && currentNode->type != Parser::Node::NodeType::ValueAccess && currentNode->type != Node::NodeType::EndGroup && static_cast<BinOpNode*>(currentNode)->opType != Token::Type::Plus && static_cast<BinOpNode*>(currentNode)->opType != Token::Type::Minus)
		{
			OpNode* factorNodeToSet = nullptr;

			if (currentNode->type == Node::NodeType::StartGroup)
			{
				//Gets all the tokens inside the brackets
				std::tuple<OpNode*, GroupNode*, std::vector<Token>*>* grp = GetGroup(info, tokens, i);
				CHECK_NODE(grp);

				//Gets the outermost node. Not always the group node as when there is a value access,
				//the value access node is the outer node
				OpNode* root = std::get<0>(*grp);
				GroupNode* group = std::get<1>(*grp);
				std::vector<Token>* groupTokens = std::get<2>(*grp);

				uint32_t groupI = 0;
				//Parses the equation of the group
				std::pair<OpNode*, std::string>* parsedEqn = ParseEquation(info, *groupTokens, groupI);
				//Sets the group's root node to the equation
				group->SetRoot(parsedEqn->first, parsedEqn->second);

				//If the root node is a value access node, each value access has to have its value type
				//set through the SetAccessValueTypes function
				if (root->type == OpNode::OpType::ValueAccess)
				{
					if (!SetAccessValueTypes(info, static_cast<ValueAccessNode*>(root)->GetAccessNode(), parsedEqn->second))
					{
						//TODO: Add respective error
						ADD_ERROR(new PlaceholderError(tokens[i].line, tokens[i].col, tokens[i].ToString()));
						return nullptr;
					}
				}

				factorNodeToSet = root;

				delete groupTokens;
			}
			//In case of cast nodes, set the current cast to this cast node
			else if (currentNode->type == Node::NodeType::Cast)
			{
				currentCast = static_cast<CastNode*>(currentNode);
			}
			//If the current node is an operation
			else if (BinOpNode* currentOp = dynamic_cast<BinOpNode*>(static_cast<OpNode*>(currentNode)))
			{
				//If a current operator node exists, set the current operator rhs to the factor node
				//and this operator (currentOp) lhs to the current operator
				if (currentOperatorNode)
				{
					currentOperatorNode->right = factorNode;
					currentOp->left = currentOperatorNode;
				}
				//Else set this operator lhs to the factor node
				else
				{
					currentOp->left = factorNode;
				}

				currentOperatorNode = currentOp;
			}
			//TODO: Check reason for this function (it was outside the loop originally)
			//If the current node is a value access
			else if (currentNode->type == Parser::Node::NodeType::ValueAccess)
			{
				//If the factor node is a group node
				if (dynamic_cast<GroupNode*>(factorNode))
				{
					ValueAccessNode* accessNode = static_cast<ValueAccessNode*>(currentNode);
					accessNode->SetRoot(factorNode);
					factorNode = accessNode;
				}
				else
				{
					ADD_ERROR(new PlaceholderError(tokens[i].line, tokens[i].col, "Invalid Value Access"));
					return nullptr;
				}
			}
			else
			{
				//If the current node is a variable
				if (currentNode->type == Node::NodeType::Variable)
				{
					VariableNode* varNode = static_cast<VariableNode*>(currentNode);
					//If the past type is empty, assume that this is the first factor so set the
					//past type to this var's type
					if (pastType.empty())
					{
						pastType = varNode->varType;
					}
					else
					{
						//If the current var's type is not equal to the past type
						if (varNode->varType != pastType)
						{
							//If the past type is scalar and the current var type is vector,
							//then set the past type to the current var's type as scalar values
							//can be added/subtracted/multiplied/divided with vectors
							if (Variable::IsScalar(pastType) && varNode->IsVector())
							{
								pastType = varNode->varType;
							}
							//If the current var isn't a scalar, then there is an error
							else if (!varNode->IsScalar())
							{
								ADD_ERROR(new PlaceholderError(tokens[i].line, tokens[i].col, "Unmatching Type"));
							}
						}
					}
				}
				//Assuming all constants are scalars (which should be correct)
				else if(currentNode->type == Node::NodeType::Constant)
				{
					ConstantNode* constNode = static_cast<ConstantNode*>(currentNode);

					//If no past type exists, set the past type to the constant's type. No need
					//to check if the past type exists and matches as the constant node is assumed
					//to be scalar therefore will also work with other value types (such as vectors)
					if (pastType.empty())
						pastType = constNode->valueType;
				}
				//TODO: Handle vector constructors
				else
				{
					PT_CORE_ERROR("Cannot handle anything but var or constant!");
				}

				factorNodeToSet = static_cast<OpNode*>(currentNode);
			}

			//If a factor node exists 
			if (factorNodeToSet)
			{
				//If a current cast exists
				if (currentCast)
				{
					//Set the current cast casting node to the factor node
					currentCast->SetCastedNode(factorNodeToSet);
					//Set the factor node to the cast node as it is now the outer node
					factorNodeToSet = currentCast;
					currentCast = nullptr;
				}

				factorNode = factorNodeToSet;
			}

			pastI = i;

			if (i >= tokens.size()) break;

			currentNode = GetNextNode(info, tokens, i);
			CHECK_NODE(currentNode);
		}

		OpNode* retNode = nullptr;

		//If a current operator node exists
		if (currentOperatorNode)
		{
			//Set the current op rhs to the factor node
			currentOperatorNode->right = factorNode;
			//Set the node to return to the current op node
			retNode = currentOperatorNode;
		}
		else
		{
			//Else just return the factor node
			retNode = factorNode;
		}

		i = pastI;

		return new std::pair<OpNode*, std::string>(retNode, pastType);
	}

	bool ShaderParser::SetAccessValueTypes(ShaderInfo& info, Variable* access, std::string accessedType)
	{
		Variable* rootAccess = access;
		std::vector<Variable*> accessValChain = { access };

		//Gets the list of all the accesses
		//So for a variable var.xyz.xxx.yyy.zzz you get a list of {zzz, yyy, xxx, xyz}
		//They are in reverse order as the outermost access value is the last access
		//(in this example it would be .zzz)
		while (rootAccess->parent)
		{
			rootAccess = rootAccess->parent;
			accessValChain.push_back(rootAccess);
		}

		std::string pastAccessedType = accessedType;

		//The access value chain is accessed in reverse as the values are in reverse
		for (int i = accessValChain.size() - 1; i >= 0; i--)
		{
			Variable* currentAccessVal = accessValChain[i];

			//If the past access type is a struct, then this access value must 
			//be a variable in the struct
			if (info.HasStruct(pastAccessedType))
			{
				//Gets the struct node of the past access type
				StructNode* structNode = info.structs[pastAccessedType];

				//If the struct contains a variable with the name of this 
				//access, then set this access type to the variable's type
				if (structNode->HasVar(currentAccessVal->varName))
				{
					currentAccessVal->varType = structNode->GetVarType(currentAccessVal->varName);
				}
				else
				{
					return false;
				}
			}
			else
			{
				//Checks the vector access type
				std::string accType = ValidateVecAccessType(pastAccessedType, currentAccessVal->varName);
				//If the access type exists (no error) then set the access type to the accType
				if (accType != "")
				{
					currentAccessVal->varType = accType;
				}
				else
				{
					return false;
				}
			}

			pastAccessedType = currentAccessVal->varType;
		}

		return true;
	}

	std::tuple<OpNode*, GroupNode*, std::vector<Token>*>* ShaderParser::GetGroup(ShaderInfo& info, std::vector<Token>& tokens, uint32_t& i)
	{
		//The group tokens to return
		std::vector<Token>* group = new std::vector<Token>();
		Token& curTok = tokens[i];

		//The number of groups found inside this group
		uint32_t foundGroups = 0;

		while (!(!foundGroups && curTok.IsType(Token::Type::RParen)))
		{
			//If there is an end of line before the end of the group there is an error
			if (curTok.IsType(Token::Type::EOL))
			{
				ADD_ERROR(new InvalidGroupError(curTok.line, curTok.col));
				return nullptr;
			}

			//If an open bracket is found, then there is a group inside this one
			if (curTok.IsType(Token::Type::LParen))
				foundGroups++;

			//If a close bracket is found, then a group inside this one has been closed
			if (curTok.IsType(Token::Type::RParen))
				foundGroups--;

			//Add the current token to the group tokens
			group->push_back(curTok);

			//Next token
			curTok = tokens[++i];
		}
		
		//Skip the next token which is a close bracket
		i++;

		//The function returns a group node
		GroupNode* grpNode = new GroupNode();
		//The function also returns a root node
		OpNode* rootNode = grpNode;

		uint32_t pastI = i;
		Node* curNode = GetNextNode(info, tokens, i);
		//If a value access node exists after the group, then the value access
		//node has to encapsulate the group node, so the root node is the value access node
		//Ex: (statement).xyz then [Access node: xyz]([Group Node])
		if (curNode->type == Node::NodeType::ValueAccess)
		{
			ValueAccessNode* accessNode = static_cast<ValueAccessNode*>(curNode);
			accessNode->SetRoot(grpNode);
			rootNode = accessNode;
		}
		else
		{
			//Else go back so that other functions can handle this node
			i = pastI;
		}

		return new std::tuple<OpNode*, GroupNode*, std::vector<Token>*>(rootNode, grpNode, group);
	}

	std::vector<Token>& ShaderParser::LexShader(const std::string& path)
	{
		Get().lastErrors.clear();
		Get().tokens.clear();
		std::ifstream stream(path, std::ios::in);
		std::string line;
		std::vector<Token>& tokens = Get().tokens;

		if (stream)
		{
			bool multilineComment = false;
			uint32_t lineNum = 1;
			//Search for type
			while (std::getline(stream, line))
			{
				std::string typeString;
				bool foundType = false;
				for (int i = 0; i < line.length(); i++)
				{
					if (multilineComment)
					{
						multilineComment = !EndMultiline(i, line);

						continue;
					}

					if (IsComment(i, line, multilineComment))
						break;

					if (multilineComment)
						continue;

					char c = line[i];
					if (!foundType && c == 't')
					{
						typeString += c;
						typeString += line[++i];
						typeString += line[++i];
						typeString += line[++i];

						if (typeString != "type")
						{
							ADD_ERROR(new MissingTypeError(lineNum, i));
							return tokens;
						}
						else
						{
							typeString = "";
							foundType = true;
							continue;
						}
					}

					if (foundType && c != ' ' && c != '\t')
					{
						typeString += c;
					}

					if (!foundType && (c != ' ' && c != '\t'))
					{
						ADD_ERROR(new MissingTypeError(lineNum, i));
						return tokens;
					}
				}

				lineNum++;

				if (foundType)
				{
					if (typeString == "Vertex")
					{
						Get().tokens.push_back(Token(Token::Type::ShaderType, "Vertex", lineNum - 1, 0));
						break;
					}
					else if (typeString == "Pixel")
					{
						Get().tokens.push_back(Token(Token::Type::ShaderType, "Pixel", lineNum - 1, 0));
						break;
					}
					else if (typeString == "Include")
					{
						Get().tokens.push_back(Token(Token::Type::ShaderType, "Include", lineNum - 1, 0));
						break;
					}
					else
					{
						ADD_ERROR(new MissingTypeError(lineNum - 1, 0));
						return tokens;
					}
				}
			}

			while (std::getline(stream, line))
			{
				for (int i = 0; i < line.length(); i++)
				{
					if (multilineComment)
					{
						multilineComment = !EndMultiline(i, line);

						continue;
					}

					char c = line[i];

					if (c == ' ' || c == '\t')
						continue;

					if (IsSymbol(c))
					{
						if (IsComment(i, line, multilineComment))
							break;

						if (multilineComment)
							continue;

						if (i > 0 && Get().symbols[c] == Token::Type::Equals)
						{
							bool foundType = true;
							switch (Get().symbols[line[i - 1]])
							{
							case Token::Type::LessThan:
								tokens[tokens.size() - 1] = Token(Token::Type::LessThanOrEqual, nullptr, lineNum, i);
								break;
							case Token::Type::GreaterThan:
								tokens[tokens.size() - 1] = Token(Token::Type::GreaterThanOrEqual, nullptr, lineNum, i);
								break;
							case Token::Type::Equals:
								tokens[tokens.size() - 1] = Token(Token::Type::EqualTo, nullptr, lineNum, i);
								break;
							case Token::Type::Exclem:
								tokens[tokens.size() - 1] = Token(Token::Type::NotEqualTo, nullptr, lineNum, i);
								break;
							default:
								foundType = false;
							}

							if (foundType)
								continue;
						}

						tokens.push_back(Token(Get().symbols[c], nullptr, lineNum, i));
						if (c == '}')
						{
							if (Get().insideConstructor)
								Get().insideConstructor = false;
							else
								Get().currentStruct = "";
						}
					}
					else if (c == ':')
					{
						i++;
						tokens.push_back(Token(Token::Type::Semantic, StrToVoidPtr(GetWord(line, i)), lineNum, i));
					}
					else
					{
						std::string word = GetWord(line, i);
						if (word == Get().currentStruct)
						{
							tokens.push_back(Token(Token::Type::Constructor, StrToVoidPtr(word), lineNum, i));
							Get().insideConstructor = true;
						}
						else if (IsVarType(word) || IsStruct(word))
						{
							tokens.push_back(Token(Token::Type::VarType, StrToVoidPtr(word), lineNum, i));
						}
						else if (word == "struct")
						{
							i++;
							Get().currentStruct = GetWord(line, i);
							Get().structs.insert(Get().currentStruct);
							tokens.push_back(Token(Token::Type::Struct, StrToVoidPtr(Get().currentStruct), lineNum, i));
						}
						else if (word == "return")
						{
							i++;
							tokens.push_back(Token(Token::Type::Return, nullptr, lineNum, i));
						}
						else if (word == "#include")
						{
							i++;
							std::string include = ReadString(line, i);
							if (include == "")
							{
								ADD_ERROR(new MistypedIncludeError(lineNum, i, line));
							}
							else
							{
								tokens.push_back(Token(Token::Type::Include, StrToVoidPtr(include), lineNum, i));
							}
						}
						else if (word.length() > 5 && word.substr(0, 5) == "this.")
						{
							char* selfVar = new char[word.length() - 4];
							strcpy(selfVar, &word.c_str()[5]);
							tokens.push_back(Token(Token::Type::SelfVar, (void*)selfVar, lineNum, i));
						}
						else if (ValidVariableName(word))
						{
							tokens.push_back(Token(Token::Type::Variable, StrToVoidPtr(word), lineNum, i));
						}
						else
						{
							bool parsed = false;
							Token::Type numType = GetNumberType(word);

							if (numType != Token::Type::Invalid)
							{
								switch (numType)
								{
								case Token::Type::Float:
									try
									{
										float* val = new float(std::stof(word.substr(0, word.length() - 1)));
										tokens.push_back(Token(Token::Type::Float, (void*)val, lineNum, i));
										parsed = true;
									}
									catch (...) {}
									break;
								case Token::Type::Double:
									try
									{
										double* val = new double(std::stod(word));
										tokens.push_back(Token(Token::Type::Double, (void*)val, lineNum, i));
										parsed = true;
									}
									catch (...) {}
									break;
								case Token::Type::Int:
									try
									{
										int* val = new int(std::stoi(word));
										tokens.push_back(Token(Token::Type::Int, (void*)val, lineNum, i));
										parsed = true;
									}
									catch (...) {}
									break;
								}
							}

							if (!parsed)
								ADD_ERROR(new UnrecognisedStatementError(lineNum, i, word));
						}
					}
				}
				lineNum++;
			}
		}

		return Get().tokens;
	}

	std::string ShaderParser::GetWord(const std::string& line, int& i)
	{
		std::string word;
		char c = line[i];

		while (c == ' ' || c == '\t')
		{
			i++;
			c = line[i];
		}

		while (c != ' ' && c != '\t' && i < line.length() && (IsNumber(word + c) || !IsSymbol(c)))
		{
			word += c;
			i++;
			c = line[i];
		}
		i--;

		return word;
	}

	std::string ShaderParser::ReadString(const std::string& line, int& i)
	{
		std::string ret;
		bool writing = false;
		while (i < line.length())
		{
			char c = line[i];

			if (writing)
			{
				if (c == '\"')
				{
					writing = false;
					break;
				}

				ret += c;
			}

			if (!writing && c == '\"')
				writing = true;
			i++;
		}

		if (writing)
			ret = "";

		return ret;
	}
}