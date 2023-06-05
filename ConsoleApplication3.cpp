#include <iostream>
#include <string>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

class DBConnectionInterface
{
public:
	DBConnectionInterface() {}
	virtual ~DBConnectionInterface() {}
	virtual bool open() = 0;
	virtual bool close() = 0;
	virtual bool execQuery(const std::string& query) = 0;
};

class DBConnection : public DBConnectionInterface
{
public:
	DBConnection() {
		std::cout << "Constructor " << this << "\n";
		descriptor = nullptr;
	}

	~DBConnection() {
		std::cout << "Destructor " << this << "\n";
		delete descriptor;
	}

	bool open() override {
		if ((descriptor == nullptr) || (*descriptor < 0))
		{
			descriptor = new int(100);
			return true;
		}
		else
			return false;
	}

	bool execQuery(const std::string& query) override {
		if (descriptor == nullptr)
			return false;
		else if (*descriptor < 0)
			return false;
		else
		{
			std::cout << "Query is " << query << "\n";
			return true;
		}
	}

	bool close() override {
		if (descriptor == nullptr)
			return false;
		else {
			*descriptor = -1;
			return true;
		}
	}

private:
	int* descriptor;
};

class MockDBConnection : public DBConnectionInterface
{
public:
	MOCK_METHOD(bool, open, (), (override));
	MOCK_METHOD(bool, close, (), (override));
	MOCK_METHOD(bool, execQuery, (const std::string& query), (override));
};

class ClassThatUsesDB
{
public:
	ClassThatUsesDB(DBConnectionInterface* connection) :
		_connection(connection) {}
	bool openConnection()
	{
		if (_connection->open())
		{
			std::cout << "Opened\n";
			return true;
		}
		else
		{
			std::cout << "Not opened\n";
			return false;
		}
	}
	bool useConnection(const std::string& query)
	{
		if (_connection->execQuery(query))
		{
			std::cout << "Execute\n";
			return true;
		}
		else
		{
			std::cout << "Not Execute\n";
			return false;
		}
	}
	bool closeConnection()
	{
		if (_connection->close())
		{
			std::cout << "Close\n";
			return true;
		}
		else
		{
			std::cout << "Failed\n";
			return false;
		}
	}
private:
	DBConnectionInterface* _connection;
};

class SomeTestSuite : public ::testing::Test
{
protected:
	void SetUp()
		{
			_dbconn = new DBConnection();
			_dbci = new ClassThatUsesDB(_dbconn);
		}
	void TearDown()
	{
		delete _dbci;
		delete _dbconn;
	}

protected:
	DBConnectionInterface* _dbconn{};
	ClassThatUsesDB* _dbci{};
};

TEST_F(SomeTestSuite, testcase1)
{
	bool test = _dbci->openConnection();
	ASSERT_EQ(test, true);
}

TEST_F(SomeTestSuite, testcase2)
{
	_dbci->openConnection();
	bool test = _dbci->useConnection("Create table");
	ASSERT_EQ(test, true);
}

TEST_F(SomeTestSuite, testcase3)
{
	bool test = _dbci->useConnection("Create table");
	ASSERT_EQ(test, false);
}

TEST_F(SomeTestSuite, testcase4)
{
	_dbci->openConnection();
	_dbci->useConnection("Create table");
	bool test = _dbci->closeConnection();
	ASSERT_EQ(test, true);
}

TEST_F(SomeTestSuite, testcase5)
{
	bool test = _dbci->closeConnection();
	ASSERT_EQ(test, false);
}

TEST_F(SomeTestSuite, testcase6)
{
	MockDBConnection mock;

	EXPECT_CALL(mock, open).WillOnce(::testing::Return(true));
	EXPECT_CALL(mock, execQuery("Create table (GMock)")).WillOnce(::testing::Return(true));
	EXPECT_CALL(mock, close).WillOnce(::testing::Return(true));

	ClassThatUsesDB dbuses(&mock);
	bool test1 = dbuses.openConnection();
	bool test2 = dbuses.useConnection("Create table (GMock)");
	bool test3 = dbuses.closeConnection();

	ASSERT_EQ(test1, true);
	ASSERT_EQ(test2, true);
	ASSERT_EQ(test3, true);
}

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
