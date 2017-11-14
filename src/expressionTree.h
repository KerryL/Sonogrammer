// File:  expressionTree.h
// Date:  5/6/2011
// Auth:  K. Loux
// Desc:  Handles user-specified mathematical operations on datasets.

#ifndef EXPRESSION_TREE_H_
#define EXPRESSION_TREE_H_

// Standard C++ headers
#include <queue>
#include <stack>
#include <string>
#include <memory>

// wxWidgets headers
#include <wx/wx.h>

/// Class for processing user-specified mathematical operations.  Uses a
/// shunting yard algorithm to build and evaluate expression trees from
/// user-specified strings.
class ExpressionTree
{
public:
	/// Solves the specified expression by simplifying and combining like
	/// terms.
	///
	/// \param expression             Expression to evaluate.
	/// \param solvedExpression [out] The result of the simplified expression.
	///
	/// \returns A description of any parsing/evaluation errors, or an empty
	///          string for success.
	std::string Solve(std::string expression, std::string &solvedExpression);

	/// Breaks the specified expression string into separate terms.
	///
	/// \param s Expression string.
	///
	/// \returns A list of separate terms.
	static wxArrayString BreakApartTerms(const wxString &s);

	/// Processes each term to extract the value of the coefficient and the
	/// power to which the variable is raised.
	///
	/// \param terms List of terms to process.
	///
	/// \returns Values that describe each term.
	static std::vector<std::pair<int, double>> FindPowersAndCoefficients(
		const wxArrayString &terms);

private:
	static const unsigned int mPrintfPrecision;

	double mXAxisFactor;

	std::queue<wxString> mOutputQueue;

	struct TermData
	{
		explicit TermData(const wxString& s) : string(s), value(0.0), useValue(false) {}
		explicit TermData(const double& v) : string(), value(v), useValue(true) {}

		const wxString string;
		const double value;
		const bool useValue;
	};

	wxString ParseExpression(const wxString &expression);
	wxString ParseNext(const wxString &expression, bool &lastWasOperator,
		unsigned int &advance, std::stack<wxString> &operatorStack);
	std::string EvaluateExpression(std::string &results);

	void ProcessOperator(std::stack<wxString> &operatorStack, const wxString &s);
	void ProcessCloseParenthese(std::stack<wxString> &operatorStack);

	static bool NextIsNumber(const wxString &s, unsigned int *stop = nullptr, const bool &lastWasOperator = true);
	static bool NextIsOperator(const wxString &s, unsigned int *stop = nullptr);
	static bool NextIsS(const wxString &s, unsigned int *stop = nullptr);

	static unsigned int FindEndOfNextTerm(const wxString &s, const unsigned int &start);
	static int GetTermPower(const wxString &s, unsigned int &start, unsigned int &end);

	bool IsLeftAssociative(const wxChar &c) const;
	bool OperatorShift(const wxString &stackString, const wxString &newString) const;

	void PopStackToQueue(std::stack<wxString> &stack);
	bool EmptyStackToQueue(std::stack<wxString> &stack);
	unsigned int GetPrecedence(const wxString &s) const;

	double ApplyOperation(const wxString &operation, const double &first, const double &second) const;

	bool PopFromStack(std::stack<TermData> &stack, wxString& string, double &value) const;

	bool EvaluateNext(const wxString &next, std::stack<TermData> &stack, wxString &errorString) const;
	bool EvaluateNumber(const wxString &number, std::stack<TermData> &stack, wxString &errorString) const;
	bool EvaluateOperator(const wxString &operation, std::stack<TermData> &stack, wxString &errorString) const;
	bool EvaluateUnaryOperator(const wxString &operation, std::stack<TermData> &stack, wxString &errorString) const;

	wxString ApplyOperation(const wxString &operation, const wxString &first, const wxString &second) const;
	wxString ApplyOperation(const wxString &operation, const wxString &first, const double &second) const;
	wxString ApplyOperation(const wxString &operation, const double &first, const wxString &second) const;

	bool ParenthesesBalanced(const wxString &expression) const;
	static bool BeginningMatchesNoCase(const wxString &s, const wxString &target, unsigned int *length = nullptr);

	wxString StringAdd(const wxString &first, const double &second) const;
	wxString StringAdd(const double &first, const wxString &second) const;

	wxString StringSubtract(const wxString &first, const double &second) const;
	wxString StringSubtract(const double &first, const wxString &second) const;

	wxString StringMultiply(const wxString &first, const double &second) const;
	wxString StringMultiply(const wxString &first, const wxString &second) const;
	wxString StringMultiply(const double &first, const wxString &second) const;

	wxString StringDivide(const double &first, const wxString &second) const;

	wxString StringPower(const double &first, const wxString &second) const;

	void AddToExpressionString(wxString &expression, const double &coefficient, const int &power) const;

	static unsigned int GetPrecision(const double &value,
		const unsigned int &significantDigits = 2,
		const bool &dropTrailingZeros = true);
};

#endif// EXPRESSION_TREE_H_
