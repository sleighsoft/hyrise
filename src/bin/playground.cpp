#include <iostream>

#include "SQLParserResult.h"
#include "SQLParser.h"
#include "util/sqlhelper.h"

int main() {
  hsql::SQLParserResult result;

  const char *query = R"(
    SELECT
      SUM(CASE
        WHEN a=5 THEN b
        WHEN b=3 THEN 8
        ELSE 9
      END)
    FROM t;
  )";

//  const auto *query = R"(SELECT O_YEAR, SUM(CASE WHEN NATION = 'BRAZIL' THEN VOLUME ELSE 0 END)/SUM(VOLUME) AS MKT_SHARE
//  FROM (SELECT datepart(yy,O_ORDERDATE) AS O_YEAR, L_EXTENDEDPRICE*(1-L_DISCOUNT) AS VOLUME, N2.N_NAME AS NATION
//  FROM "PART", SUPPLIER, LINEITEM, ORDERS, CUSTOMER, NATION N1, NATION N2, REGION
//  WHERE P_PARTKEY = L_PARTKEY AND S_SUPPKEY = L_SUPPKEY AND L_ORDERKEY = O_ORDERKEY
//  AND O_CUSTKEY = C_CUSTKEY AND C_NATIONKEY = N1.N_NATIONKEY AND
//  N1.N_REGIONKEY = R_REGIONKEY AND R_NAME = 'AMERICA' AND S_NATIONKEY = N2.N_NATIONKEY
//  AND O_ORDERDATE BETWEEN '1995-01-01' AND '1996-12-31' AND P_TYPE= 'ECONOMY ANODIZED STEEL') AS ALL_NATIONS
//  GROUP BY O_YEAR
//  ORDER BY O_YEAR)";


  hsql::SQLParser::parse(query, &result);

  if (!result.isValid()) {
    std::cerr << "Error compiling: " << result.errorMsg() << std::endl;
    return 1;
  }


  hsql::printStatementInfo(result.getStatements()[0]);

  std::cout << "Hello world!" << std::endl;
  return 0;
}
