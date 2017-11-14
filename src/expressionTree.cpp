// File:  expressionTree.cpp
// Date:  5/6/2011
// Auth:  K. Loux
// Desc:  Handles user-specified mathematical operations on datasets.

// wxWidgets headers
#include <wx/wx.h>

// Local headers
#include "expressionTree.h"

// Standard C++ headers
#include <sstream>

//=============================================================================
// Class:			ExpressionTree
// Function:		Constant Declarations
//
// Description:		Constant declarations for ExpressionTree class.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//=============================================================================
const unsigned int ExpressionTree::mPrintfPrecision = 15;

//=============================================================================
// Class:			ExpressionTree
// Function:		Solve
//
// Description:		Main solving method for the tree.
//
// Input Arguments:
//		expression			= std::string containing the expression to parse
//
// Output Arguments:
//		solvedExpression	= std::string& containing the evaluated data
//
// Return Value:
//		std::string, empty for success, error string if unsuccessful
//
//=============================================================================
std::string ExpressionTree::Solve(std::string expression,
	std::string &solvedExpression)
{
	if (!ParenthesesBalanced(expression))
		return "Imbalanced parentheses!";

	std::string errorString;
	errorString = ParseExpression(expression).c_str();

	if (!errorString.empty())
		return errorString;

	errorString = EvaluateExpression(solvedExpression);

	return errorString;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		ParenthesesBalanced
//
// Description:		Checks to see if the expression has balanced parentheses.
//
// Input Arguments:
//		expression	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if parentheses are balanced, false otherwise
//
//=============================================================================
bool ExpressionTree::ParenthesesBalanced(const wxString &expression) const
{
	unsigned int leftCount(0), rightCount(0);
	int location = expression.find(_T("("));

	while (location != wxNOT_FOUND)
	{
		++leftCount;
		location = expression.find(_T("("), location + 1);
	}

	location = expression.find(_T(")"));

	while (location != wxNOT_FOUND)
	{
		++rightCount;
		location = expression.find(_T(")"), location + 1);
	}

	if (leftCount != rightCount)
		return false;

	return true;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		ParseExpression
//
// Description:		Parses the expression and produces a queue of Reverse
//					Polish Notation values and operations.  Implements the
//					shunting-yard algorithm as described by Wikipedia.
//
// Input Arguments:
//		expression	= const wxString& to be parsed
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString containing error descriptions or an empty string on success
//
//=============================================================================
wxString ExpressionTree::ParseExpression(const wxString &expression)
{
	std::stack<wxString> operatorStack;
	unsigned int i, advance;
	bool lastWasOperator(true);
	wxString errorString;

	for (i = 0; i < expression.Len(); ++i)
	{
		if (expression.Mid(i, 1).Trim().IsEmpty())
			continue;

		errorString = ParseNext(expression.Mid(i), lastWasOperator,
			advance, operatorStack);
		if (!errorString.IsEmpty())
			return errorString;
		i += advance - 1;
	}

	if (!EmptyStackToQueue(operatorStack))
		errorString = _T("Imbalanced parentheses!");

	return errorString;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		ParseNext
//
// Description:		Parses the expression and processes the next item.
//
// Input Arguments:
//		expression	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString containing any errors
//
//=============================================================================
wxString ExpressionTree::ParseNext(const wxString &expression,
	bool &lastWasOperator, unsigned int &advance,
	std::stack<wxString> &operatorStack)
{
	bool thisWasOperator(false);
	if (NextIsNumber(expression, &advance, lastWasOperator))
		mOutputQueue.push(expression.Mid(0, advance));
	else if (NextIsS(expression, &advance))
		mOutputQueue.push(expression.Mid(0, advance));
	else if (NextIsOperator(expression, &advance))
	{
		ProcessOperator(operatorStack, expression.Mid(0, advance));
		thisWasOperator = true;
	}
	else if (expression[0] == '(')
	{
		if (!lastWasOperator)
			operatorStack.push(_T("*"));
		operatorStack.push(expression[0]);
		advance = 1;
		thisWasOperator = true;
	}
	else if (expression[0] == ')')
	{
		ProcessCloseParenthese(operatorStack);
		advance = 1;
	}
	else
		return _T("Unrecognized character:  '") + expression.Mid(0, 1) + _T("'.");
	lastWasOperator = thisWasOperator;
	return wxEmptyString;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		ProcessOperator
//
// Description:		Processes the next operator in the expression, adding it
//					to the appropriate stack.  This method enforces the order
//					of operations.
//
// Input Arguments:
//		operatorStack	= std::stack<wxString>&
//		s				= const wxString& representing the next operator
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//=============================================================================
void ExpressionTree::ProcessOperator(std::stack<wxString> &operatorStack,
	const wxString &s)
{
	// Handle operator precedence
	while (!operatorStack.empty())
	{
		if ((!NextIsOperator(operatorStack.top()) ||
			!OperatorShift(operatorStack.top(), s)))
			break;
		PopStackToQueue(operatorStack);
	}
	operatorStack.push(s);
}

//=============================================================================
// Class:			ExpressionTree
// Function:		ProcessCloseParenthese
//
// Description:		Adjusts the stacks in response to encountering a close
//					parenthese.
//
// Input Arguments:
//		operatorStack	= std::stack<wxString>&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//=============================================================================
void ExpressionTree::ProcessCloseParenthese(std::stack<wxString> &operatorStack)
{
	while (!operatorStack.empty())
	{
		if (operatorStack.top().Cmp(_T("(")) == 0)
			break;
		PopStackToQueue(operatorStack);
	}

	if (operatorStack.empty())
	{
		assert(false);
		// Should never happen due to prior parenthese balance checks
		//return _T("Imbalanced parentheses!");
	}

	operatorStack.pop();
	if (!operatorStack.empty())
	{
		/*if (NextIsFunction(operatorStack.top()))
			PopStackToQueue(operatorStack);*/
	}
}

//=============================================================================
// Class:			ExpressionTree
// Function:		EvaluateExpression
//
// Description:		Evaluates the expression in the queue using Reverse Polish
//					Notation.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		results	= std::string&
//
// Return Value:
//		std::string containing a description of any errors, or empty string on
//		success
//
//=============================================================================
std::string ExpressionTree::EvaluateExpression(std::string &results)
{
	wxString next, errorString;

	std::stack<TermData> stack;

	while (!mOutputQueue.empty())
	{
		next = mOutputQueue.front();
		mOutputQueue.pop();

		if (!EvaluateNext(next, stack, errorString))
			return std::string(errorString.mb_str());
	}

	if (stack.size() > 1)
		return "Not enough operators!";

	if (stack.top().useValue)
		results = wxString::Format("%0.*f",
			GetPrecision(stack.top().value, mPrintfPrecision),
			stack.top().value).mb_str();
	else
		results = stack.top().string;

	return "";
}

//=============================================================================
// Class:			ExpressionTree
// Function:		PopStackToQueue
//
// Description:		Removes the top entry of the stack and puts it in the queue.
//
// Input Arguments:
//		stack	= std::stack<wxString>& to be popped
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//=============================================================================
void ExpressionTree::PopStackToQueue(std::stack<wxString> &stack)
{
	mOutputQueue.push(stack.top());
	stack.pop();
}

//=============================================================================
// Class:			ExpressionTree
// Function:		EmptyStackToQueue
//
// Description:		Empties the contents of the stack into the queue.
//
// Input Arguments:
//		stack	= std::stack<wxString>& to be emptied
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise (imbalance parentheses)
//
//=============================================================================
bool ExpressionTree::EmptyStackToQueue(std::stack<wxString> &stack)
{
	while (!stack.empty())
	{
		if (stack.top().Cmp(_T("(")) == 0)
			return false;
		PopStackToQueue(stack);
	}

	return true;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		NextIsNumber
//
// Description:		Determines if the next portion of the expression is a
//					number. Some cleverness is required to tell the difference
//					between a minus sign and a negative sign (minus sign would
//					return false).
//
// Input Arguments:
//		s				= const wxString& containing the expression
//		lastWasOperator	= const bool& indicating whether or not the last thing
//						  on the stack is an operator
//
// Output Arguments:
//		stop	= unsigned int* (optional) indicating length of number
//
// Return Value:
//		bool, true if a number is next in the expression
//
//=============================================================================
bool ExpressionTree::NextIsNumber(const wxString &s, unsigned int *stop,
	const bool &lastWasOperator)
{
	if (s.Len() == 0)
		return false;

	bool foundDecimal = s[0] == '.';
	if (foundDecimal ||
		(int(s[0]) >= int('0') && int(s[0]) <= int('9')) ||
		(s[0] == '-' && lastWasOperator && NextIsNumber(s.Mid(1), nullptr, false)))
	{
		unsigned int i;
		for (i = 1; i < s.Len(); ++i)
		{
			if (s[i] == '.')
			{
				if (foundDecimal)
					return false;
				foundDecimal = true;
			}
			else if (int(s[i]) < int('0') || int(s[i]) > int('9'))
				break;
		}

		if (stop)
			*stop = i;
		return true;
	}

	return false;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		NextIsS
//
// Description:		Determines if the next portion of the expression is
//					complex frequency (s) or discrete time (z).
//
// Input Arguments:
//		s		= const wxString& containing the expression
//
// Output Arguments:
//		stop	= unsigned int* (optional) indicating length
//
// Return Value:
//		bool, true if a dataset is next in the expression
//
//=============================================================================
bool ExpressionTree::NextIsS(const wxString &s, unsigned int *stop)
{
	if (s[0] == 's' || s[0] == 'z')
	{
		if (s.length() > 1 &&
			((s[1] >= 'a' && s[1] <= 'z') ||
			((s[1] >= 'A' && s[1] <= 'Z'))))
			return false;
		
		if (stop)
			*stop = 1;
		return true;
	}

	return false;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		NextIsOperator
//
// Description:		Determines if the next portion of the expression is an
//					operator.
//
// Input Arguments:
//		s		= const wxString& containing the expression
//
// Output Arguments:
//		stop	= unsigned int* (optional) indicating length of operator
//
// Return Value:
//		bool, true if an operator is next in the expression
//
//=============================================================================
bool ExpressionTree::NextIsOperator(const wxString &s, unsigned int *stop)
{
	if (s.Len() == 0)
		return false;

	if (s[0] == '+' ||// From least precedence
		s[0] == '-' ||
		s[0] == '*' ||
		s[0] == '/' ||
		s[0] == '%' ||
		s[0] == '^')// To most precedence
	{
		if (stop)
			*stop = 1;
		return true;
	}

	return false;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		OperatorShift
//
// Description:		Determines if the new operator requires a shift in
//					operator placement.
//
// Input Arguments:
//		stackString	= const wxString& containing the expression
//		newString	= const wxString& containing the expression
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if shifting needs to occur
//
//=============================================================================
bool ExpressionTree::OperatorShift(const wxString &stackString,
	const wxString &newString) const
{
	unsigned int stackPrecedence = GetPrecedence(stackString);
	unsigned int newPrecedence = GetPrecedence(newString);

	if (stackPrecedence == 0 || newPrecedence == 0)
		return false;

	if (IsLeftAssociative(newString[0]))
	{
		if (newPrecedence <= stackPrecedence)
			return true;
	}
	else if (newPrecedence < stackPrecedence)
		return true;

	return false;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		GetPrecedence
//
// Description:		Determines the precedence of the specified operator
//					(higher values are performed first)
//
// Input Arguments:
//		s	= const wxString& containing the operator
//
// Output Arguments:
//		None
//
// Return Value:
//		unsigned int representing the precedence
//
//=============================================================================
unsigned int ExpressionTree::GetPrecedence(const wxString &s) const
{
	if (s.Len() != 1)
		return 0;

	if (s[0] == '+' ||
		s[0] == '-')
		return 2;
	else if (s[0] == '*' ||
		s[0] == '/' ||
		s[0] == '%')
		return 3;
	else if (s[0] == '^')
		return 4;

	return 0;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		IsLeftAssociative
//
// Description:		Determines if the specified operator is left or right
//					associative.
//
// Input Arguments:
//		c	= const wxChar&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if left associative
//
//=============================================================================
bool ExpressionTree::IsLeftAssociative(const wxChar &c) const
{
	switch (c)
	{
	case '^':
		return false;

	default:
		return true;
	}
}

//=============================================================================
// Class:			ExpressionTree
// Function:		PopFromStack
//
// Description:		Pops the next value from the top of the appropriate stack.
//
// Input Arguments:
//		stack		= std::stack<TermData>&
//
// Output Arguments:
//		string		= wxString&
//		value		= double&
//
// Return Value:
//		bool, true if a value was popped, false otherwise
//
//=============================================================================
bool ExpressionTree::PopFromStack(std::stack<TermData> &stack,
	wxString& string, double &value) const
{
	const bool useValue(stack.top().useValue);

	if (stack.top().useValue)
		value = stack.top().value;
	else
		string = stack.top().string;

	stack.pop();

	return useValue;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		ApplyOperation
//
// Description:		Applies the specified operation to the specified operands.
//
// Input Arguments:
//		operation	= const wxString& describing the function to apply
//		first		= const double&
//		second		= const double&
//
// Output Arguments:
//		None
//
// Return Value:
//		double containing the result of the operation
//
//=============================================================================
double ExpressionTree::ApplyOperation(const wxString &operation,
	const double &first, const double &second) const
{
	if (operation.Cmp(_T("+")) == 0)
		return second + first;
	else if (operation.Cmp(_T("-")) == 0)
		return second - first;
	else if (operation.Cmp(_T("*")) == 0)
		return second * first;
	else if (operation.Cmp(_T("/")) == 0)
		return second / first;
	else if (operation.Cmp(_T("%")) == 0)
		return fmod(second, first);
	else if (operation.Cmp(_T("^")) == 0)
		return pow(second, first);

	assert(false);
	return 0.0;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		ApplyOperation
//
// Description:		Applies the specified operation to the specified operands.
//
// Input Arguments:
//		operation	= const wxString& describing the function to apply
//		first		= const wxString&
//		second		= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString containing the result of the operation
//
//=============================================================================
wxString ExpressionTree::ApplyOperation(const wxString &operation,
	const wxString &first, const wxString &second) const
{
	/*if (operation.Cmp(_T("+")) == 0)
		return second + first;
	else if (operation.Cmp(_T("-")) == 0)
		return second - first;
	else */if (operation.Cmp(_T("*")) == 0)
		return StringMultiply(first, second);
	return second + operation + first;

/*	assert(false);
	return "";*/
}

//=============================================================================
// Class:			ExpressionTree
// Function:		ApplyOperation
//
// Description:		Applies the specified operation to the specified operands.
//
// Input Arguments:
//		operation	= const wxString& describing the function to apply
//		first		= const wxString&
//		second		= const double&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString containing the result of the operation
//
//=============================================================================
wxString ExpressionTree::ApplyOperation(const wxString &operation,
	const wxString &first, const double &second) const
{
	if (operation.Cmp(_T("+")) == 0)
		return StringAdd(first, second);
	else if (operation.Cmp(_T("-")) == 0)
		return StringSubtract(first, second);
	else if (operation.Cmp(_T("*")) == 0)
		return StringMultiply(first, second);

	assert(false);
	return "";
}

//=============================================================================
// Class:			ExpressionTree
// Function:		ApplyOperation
//
// Description:		Applies the specified operation to the specified operands.
//
// Input Arguments:
//		operation	= const wxString& describing the function to apply
//		first		= const double&
//		second		= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString containing the result of the operation
//
//=============================================================================
wxString ExpressionTree::ApplyOperation(const wxString &operation,
	const double &first, const wxString &second) const
{
	if (operation.Cmp(_T("+")) == 0)
		return StringAdd(first, second);
	else if (operation.Cmp(_T("-")) == 0)
		return StringSubtract(first, second);
	else if (operation.Cmp(_T("*")) == 0)
		return StringMultiply(first, second);
	else if (operation.Cmp(_T("/")) == 0)
		return StringDivide(first, second);
	else if (operation.Cmp(_T("^")) == 0)
		return StringPower(first, second);

	assert(false);
	return "";
}

//=============================================================================
// Class:			ExpressionTree
// Function:		EvaluateOperator
//
// Description:		Evaluates the operator specified.
//
// Input Arguments:
//		operator	= const wxString& describing the function to apply
//		stack		= std::stack<TermData>&
//
// Output Arguments:
//		errorString	= wxString&
//
// Return Value:
//		bool, true for success, false otherwise
//
//=============================================================================
bool ExpressionTree::EvaluateOperator(const wxString &operation,
	std::stack<TermData> &stack, wxString &errorString) const
{
	double value1, value2;
	wxString string1, string2;

	if (stack.size() < 2)
		return EvaluateUnaryOperator(operation, stack, errorString);
	else if (PopFromStack(stack, string1, value1))
	{
		if (PopFromStack(stack, string2, value2))
			stack.push(TermData(ApplyOperation(operation, value1, value2)));
		else
			stack.push(TermData(ApplyOperation(operation, value1, string2)));
	}
	else if (PopFromStack(stack, string2, value2))
		stack.push(TermData(ApplyOperation(operation, string1, value2)));
	else
		stack.push(TermData(ApplyOperation(operation, string1, string2)));

	return true;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		EvaluateUnaryOperator
//
// Description:		Evaluates the operator specified.  The only unary operator
//					we recognize is minus (negation).
//
// Input Arguments:
//		operator	= const wxString& describing the function to apply
//		stack		= std::stack<TermData>&
//
// Output Arguments:
//		errorString	= wxString&
//
// Return Value:
//		bool, true for success, false otherwise
//
//=============================================================================
bool ExpressionTree::EvaluateUnaryOperator(const wxString &operation,
	std::stack<TermData> &stack, wxString &errorString) const
{
	if (operation.Cmp(_T("-")) != 0)
	{
		errorString = _T("Attempting to apply operator without two operands!");
		return false;
	}

	double value;
	wxString string;
	if (PopFromStack(stack, string, value))
		stack.push(TermData(ApplyOperation(_T("*"), -1.0, value)));
	else
		stack.push(TermData(ApplyOperation(_T("*"), -1.0, string)));

	return true;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		EvaluateNumber
//
// Description:		Evaluates the number specified.
//
// Input Arguments:
//		number		= const wxString& describing the function to apply
//		stack		= std::stack<TermData>&
//
// Output Arguments:
//		errorString	= wxString&
//
// Return Value:
//		bool, true for success, false otherwise
//
//=============================================================================
bool ExpressionTree::EvaluateNumber(const wxString &number,
	std::stack<TermData> &stack, wxString &errorString) const
{
	double value;
	if (!number.ToDouble(&value))
	{
		errorString = _T("Could not convert ") + number + _T(" to a number.");
		return false;
	}

	stack.push(TermData(value));

	return true;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		EvaluateNext
//
// Description:		Determines how to evaluate the specified term and takes
//					appropriate action.
//
// Input Arguments:
//		next		= const wxString&
//		stack		= std::stack<TermData>&
//
// Output Arguments:
//		errorString	= wxString&
//
// Return Value:
//		bool, true for valid operation, false otherwise
//
//=============================================================================
bool ExpressionTree::EvaluateNext(const wxString &next,
	std::stack<TermData> &stack, wxString &errorString) const
{
	if (NextIsNumber(next))
		return EvaluateNumber(next, stack, errorString);
	else if(NextIsOperator(next))
		return EvaluateOperator(next, stack, errorString);
	else if (NextIsS(next))
	{
		stack.push(TermData(next));
		return true;
	}
	else
		errorString = _T("Unable to evaluate '") + next + _T("'.");

	return false;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		BeginningMatchesNoCase
//
// Description:		Determines if the target string matches the beginning of
//					string s.  Populates the length argument if provided and
//					if a match is found.
//
// Input Arguments:
//		s		= const wxString&
//		target	= const wxString&
//
// Output Arguments:
//		length	= unsigned int* (optional)
//
// Return Value:
//		bool, true for match
//
//=============================================================================
bool ExpressionTree::BeginningMatchesNoCase(const wxString &s,
	const wxString &target, unsigned int *length)
{
	if (s.Len() < target.Len())
		return false;

	if (s.Mid(0, target.Len()).CmpNoCase(target) != 0)
		return false;

	if (length)
		*length = target.Len();

	return true;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		StringAdd
//
// Description:		Performs arithmatic on the arguments, returns a string.
//
// Input Arguments:
//		first	= const wxString&
//		second	= const double&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//=============================================================================
wxString ExpressionTree::StringAdd(const wxString &first, const double &second) const
{
	return wxString::Format("%0.*f+%s",
		GetPrecision(second, mPrintfPrecision), second, first.c_str());
}

//=============================================================================
// Class:			ExpressionTree
// Function:		StringAdd
//
// Description:		Performs arithmatic on the arguments, returns a string.
//
// Input Arguments:
//		first	= const double&
//		second	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//=============================================================================
wxString ExpressionTree::StringAdd(const double &first, const wxString &second) const
{
	return wxString::Format("%s+%0.*f", second.c_str(),
		GetPrecision(first, mPrintfPrecision), first);
}

//=============================================================================
// Class:			ExpressionTree
// Function:		StringSubtract
//
// Description:		Performs arithmatic on the arguments, returns a string.
//
// Input Arguments:
//		first	= const wxString&
//		second	= const double&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//=============================================================================
wxString ExpressionTree::StringSubtract(const wxString &first, const double &second) const
{
	return wxString::Format("%0.*f-%s",
		GetPrecision(second, mPrintfPrecision), second, first.c_str());
}

//=============================================================================
// Class:			ExpressionTree
// Function:		StringSubtract
//
// Description:		Performs arithmatic on the arguments, returns a string.
//
// Input Arguments:
//		first	= const double&
//		second	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//=============================================================================
wxString ExpressionTree::StringSubtract(const double &first, const wxString &second) const
{
	return wxString::Format("%s-%0.*f", second.c_str(),
		GetPrecision(first, mPrintfPrecision), first);
}

//=============================================================================
// Class:			ExpressionTree
// Function:		StringMultiply
//
// Description:		Performs arithmatic on the arguments, returns a string.
//
// Input Arguments:
//		first	= const wxString&
//		second	= const double&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//=============================================================================
wxString ExpressionTree::StringMultiply(const wxString &first,
	const double &second) const
{
	std::vector<std::pair<int, double>> terms(FindPowersAndCoefficients(
		BreakApartTerms(first)));
	wxString expression;
	for (const auto& term : terms)
		AddToExpressionString(expression, term.second * second, term.first);

	return expression;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		StringMultiply
//
// Description:		Performs arithmatic on the arguments, returns a string.
//
// Input Arguments:
//		first	= const wxString&
//		second	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//=============================================================================
wxString ExpressionTree::StringMultiply(const wxString &first,
	const wxString &second) const
{
	std::vector<std::pair<int, double>> firstTerms(
		FindPowersAndCoefficients(BreakApartTerms(first)));
	std::vector<std::pair<int, double>> secondTerms(
		FindPowersAndCoefficients(BreakApartTerms(second)));
	std::vector<std::pair<int, double>> terms;
	for (const auto& firstTerm : firstTerms)
	{
		for (const auto& secondTerm : secondTerms)
			terms.push_back(std::pair<int, double>(
				firstTerm.first + secondTerm.first,
				firstTerm.second * secondTerm.second));
	}

	wxString expression;
	for (const auto& term : terms)
		AddToExpressionString(expression, term.second, term.first);

	return expression;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		StringMultiply
//
// Description:		Performs arithmatic on the arguments, returns a string.
//
// Input Arguments:
//		first	= const double&
//		second	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//=============================================================================
wxString ExpressionTree::StringMultiply(const double &first, const wxString &second) const
{
	return StringMultiply(second, first);
}

//=============================================================================
// Class:			ExpressionTree
// Function:		StringDivide
//
// Description:		Performs arithmatic on the arguments, returns a string.
//
// Input Arguments:
//		first	= const double&
//		second	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//=============================================================================
wxString ExpressionTree::StringDivide(const double &first, const wxString &second) const
{
	return StringMultiply(second, 1.0 / first);
}

//=============================================================================
// Class:			ExpressionTree
// Function:		StringPower
//
// Description:		Performs arithmatic on the arguments, returns a string.
//					For positive powers, expand and do the multiplication.
//					For negative powers (i.e. z-domain stuff), add them to the
//					string.  Assumes exponent is an integer.
//
// Input Arguments:
//		first	= const double&
//		second	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString
//
//=============================================================================
wxString ExpressionTree::StringPower(const double &first, const wxString &second) const
{
	if (first < 0.0)
		return wxString::Format("%s^%i", second.c_str(), static_cast<int>(first));

	wxString result(second);
	unsigned int i;
	for (i = 1; i < static_cast<unsigned int>(first); ++i)
		result = StringMultiply(result, second);

	return result;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		BreakApartTerms
//
// Description:		Breaks apart all the terms in the string expression.  Be
//					wary of negative signs preceded by another operator!
//
// Input Arguments:
//		s	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		wxArrayString
//
//=============================================================================
wxArrayString ExpressionTree::BreakApartTerms(const wxString &s)
{
	wxArrayString terms;
	unsigned int start(0), end(0);
	while (end != static_cast<unsigned int>(wxNOT_FOUND))
	{
		end = FindEndOfNextTerm(s, start);

		if (start > 0 && s.Mid(start - 1, 1).Cmp(_T("-")) == 0)
		{
			if (end != static_cast<unsigned int>(wxNOT_FOUND))
				terms.Add(s.Mid(start - 1, end + 1));
			else
				terms.Add(s.Mid(start - 1));
		}
		else
			terms.Add(s.Mid(start, end));

		start += end + 1;
	}

	return terms;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		FindEndOfNextTerm
//
// Description:		Finds the end of the next term in the string.
//
// Input Arguments:
//		s		= const wxString&
//		start	= const unsigned int&
//
// Output Arguments:
//		None
//
// Return Value:
//		unsigned int
//
//=============================================================================
unsigned int ExpressionTree::FindEndOfNextTerm(const wxString &s, const unsigned int &start)
{
	unsigned int end, plusEnd, minusEnd;

	plusEnd = s.Mid(start).Find('+');
	minusEnd = s.Mid(start).Find('-');

	if (minusEnd < plusEnd && start + minusEnd > 0 && NextIsOperator(s[start + minusEnd - 1]))
	{
		unsigned int nextMinus = s.Mid(start + minusEnd + 1).Find('-');
		if (nextMinus != static_cast<unsigned int>(wxNOT_FOUND))
			minusEnd += nextMinus + 1;
		else
			minusEnd = nextMinus;
	}
	end = std::min(plusEnd, minusEnd);

	if (end != static_cast<unsigned int>(wxNOT_FOUND) && NextIsOperator(s.Mid(start + end - 1)))
	{
		plusEnd = s.Mid(start + end).Find('+');
		minusEnd = s.Mid(start + end).Find('-');
		end += std::min(plusEnd, minusEnd);
	}

	return end;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		FindPowersAndCoefficients
//
// Description:		Breaks a (previously separated) set of terms into a coefficient
//					and a power of the algebraic variable.
//
// Input Arguments:
//		terms	= const wxArrayString&
//
// Output Arguments:
//		None
//
// Return Value:
//		std::vector<std::pair<int, double>>
//
//=============================================================================
std::vector<std::pair<int, double>> ExpressionTree::FindPowersAndCoefficients(const wxArrayString &terms)
{
	std::vector<std::pair<int, double>> processedTerms;
	double temp;
	for (const auto& term : terms)
	{
		int count(0);
		unsigned int start(0);
		unsigned int end(0);
		double coefficient(1.0);
		while (end != static_cast<unsigned int>(wxNOT_FOUND))
		{
			end = term.Mid(start).Find('*');
			if (term.Mid(start, end).ToDouble(&temp))
				coefficient = temp;
			else
			{
				if (term[0] == '-' && coefficient == 1.0)
				{
					coefficient = -1.0;
					++start;
					if (end != static_cast<unsigned int>(wxNOT_FOUND))
						++end;
				}

				count += GetTermPower(term.Mid(start), start, end);
			}
			start += end + 1;
		}

		processedTerms.push_back(std::pair<int, double>(count, coefficient));
	}

	return processedTerms;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		GetTermPower
//
// Description:		Returns the value of the power for the specified term
//					(power of s or z).
//
// Input Arguments:
//		s		= const wxString&
//
// Output Arguments:
//		start	= unsigned int&
//		end		= unsigned int&
//
// Return Value:
//		int
//
//=============================================================================
int ExpressionTree::GetTermPower(const wxString &s, unsigned int &start, unsigned int &end)
{
	long power;
	if (s[0] == 's' || s[0] == 'z')
	{
		power = s.Find('^');
		if (power == wxNOT_FOUND)
			return 1;
		else
		{
			start += power + 1;
			end = s.Find('*');
			if (s.Mid(power + 1, end).ToLong(&power))
				return power;
		}
	}

	return 0;
}

//=============================================================================
// Class:			ExpressionTree
// Function:		AddToExpressionString
//
// Description:		Adds the next term to the string.  Handles signed terms,
//					cleans up for terms with coefficient == 1.0, etc.
//
// Input Arguments:
//		coefficient	= const double&
//		power		= const int&
//
// Output Arguments:
//		expression	= wxString&
//
// Return Value:
//		None
//
//=============================================================================
void ExpressionTree::AddToExpressionString(wxString &expression,
	const double &coefficient, const int &power) const
{
	if (coefficient == 1.0 && power != 0)
	{
		if (!expression.IsEmpty())
			expression.Append(_T("+"));
		if (power == 1)
				expression.Append(_T("s"));
		else
			expression.Append(wxString::Format("s^%i", power));
	}
	else if (expression.IsEmpty())
	{
		if (power == 0)
			expression.Printf("%0.*f", GetPrecision(coefficient, mPrintfPrecision), coefficient);
		else if (power == 1)
			expression.Printf("%0.*f*s", GetPrecision(coefficient, mPrintfPrecision), coefficient);
		else
			expression.Printf("%0.*f*s^%i", GetPrecision(coefficient, mPrintfPrecision), coefficient, power);
	}
	else
	{
		if (power == 0)
			expression.Append(wxString::Format("%+0.*f",
				GetPrecision(coefficient, mPrintfPrecision), coefficient));
		else if (power == 1)
			expression.Append(wxString::Format("%+0.*f*s",
				GetPrecision(coefficient, mPrintfPrecision), coefficient));
		else
			expression.Append(wxString::Format("%+0.*f*s^%i",
				GetPrecision(coefficient, mPrintfPrecision), coefficient, power));
	}
}

//=============================================================================
// Namespace:		ExpressionTree
// Function:		GetPrecision
//
// Description:		Determines the best number of digits after the decimal place
//					for a string representation of the specified value (for
//					use with printf-style %0.*f formatting.
//
// Input Arguments:
//		value				= const double&
//		significantDigits	= const unsigned int&
//		dropTrailingZeros	= const bool&
//
// Output Arguments:
//		None
//
// Return Value:
//		unsigned int
//
//=============================================================================
unsigned int ExpressionTree::GetPrecision(const double &value,
	const unsigned int &significantDigits, const bool &dropTrailingZeros)
{
	int precision(significantDigits - static_cast<unsigned int>(floor(log10(value)) - 1));
	if (precision < 0)
		precision = 0;
	if (!dropTrailingZeros)
		return precision;

	std::ostringstream ss;
	ss.precision(precision);
	ss << value;

	std::string number(ss.str());
	unsigned int i;
	for (i = number.size() - 1; i > 0; --i)
	{
		if (number[i] == '0')
			--precision;
		else
			break;
	}

	if (precision < 0)
		precision = 0;

	return precision;
}
