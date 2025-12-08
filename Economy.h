#pragma once

#include <vector>

class Commodity {
public:
	float supply;
	float price;

	float landAlloc;
	float labourAlloc;
	float capitalAlloc;

	float relativeDemand;
	float monetaryDemand;

	float marginalUtility;
	float marginalUtilityFunction(float consumptionPerCapita) {
		return 1.0f;
	}
};

class Economy {
public:

	int population;

	float aggregateIncome;
	float goingWorkdayLength;

	std::vector<Commodity> commodities;

	float marginalDisUtilityLabour(float goingWorkdayLength) {
		return 1.0f;
	}

	void update() {

		float labourSupply = population * goingWorkdayLength;

		// Production
		for (Commodity& commodity : commodities) {
			commodity.supply = commodity.labourAlloc * labourSupply;
		}

		// Spending
		for (Commodity& commodity : commodities) {
			commodity.monetaryDemand = commodity.relativeDemand * aggregateIncome;
		}

		// Prices
		for (Commodity& commodity : commodities) {
			commodity.price = commodity.monetaryDemand / commodity.supply;
		}

		// Aggregate Income
		float aggregateIncome = 0.0f;
		for (Commodity& commodity : commodities) {
			aggregateIncome += commodity.monetaryDemand;
		}

		// Marginal Utility
		for (Commodity& commodity : commodities) {
			float consumptionPerCapita = commodity.supply / population;
			commodity.marginalUtility = commodity.marginalUtilityFunction(consumptionPerCapita) / commodity.price;
		}

		// Adjust Workday Length
		float nominalWageRate = aggregateIncome / goingWorkdayLength;
		float highestMarginalUtility = 0.0f;
		for (Commodity& commodity : commodities) {
			if (commodity.marginalUtility > highestMarginalUtility) {
				highestMarginalUtility = commodity.marginalUtility;
			}
		}
		float marginalDisUtilityLabourIncome = marginalDisUtilityLabour(goingWorkdayLength) / nominalWageRate;
		if (marginalDisUtilityLabourIncome > highestMarginalUtility) {
			goingWorkdayLength *= 0.999f;
		}
		else {
			goingWorkdayLength *= 1.001f;
		}

		// Adjust Demand
		std::vector<float> relativeDemandVector;
		std::vector<float> commodityMarginalUtilities;
		for (Commodity& commodity : commodities) {
			relativeDemandVector.push_back(commodity.relativeDemand);
			commodityMarginalUtilities.push_back(commodity.marginalUtility);
		}
		int indexMostValue = 0;
		float currUtility = 0.0f;
		for (int i = 0; i < commodities.size(); i++) {
			Commodity& commodity = commodities[i];
			if (commodity.marginalUtility > currUtility) {
				currUtility = commodity.marginalUtility;
				indexMostValue = i;
			}
		}
		relativeDemandVector[indexMostValue] += 0.001;
		float demandSum = 0.0f;
		for (float& demand : relativeDemandVector) {
			demandSum += demand;
		}
		for (float& demand : relativeDemandVector) {
			demand = demand / demandSum;
		}
		for (int i = 0; i < commodities.size(); i++) {
			Commodity& commodity = commodities[i];
			commodity.relativeDemand = relativeDemandVector[i];
		}
	}
};

