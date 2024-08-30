#include "MarketJobs.h"
#include <numeric>
#include <ranges>


V3::MarketJobs::MarketJobs(const std::vector<std::pair<std::string, int>>& manorHouseRoster): manorHouseRoster(manorHouseRoster)
{
}

// Returns a map of each job as a percentage of all jobs in the market.
std::map<std::string, double> V3::MarketJobs::getJobBreakdown() const
{
	std::map<std::string, double> jobBreakdown;

	const auto thePop = population < 1 ? 1 : population;
	for (const auto& [job, amount]: jobCounts)
	{
		jobBreakdown[job] = amount / thePop;
	}

	return jobBreakdown;
}

void V3::MarketJobs::createJobs(const PopType& popType, double amount, const double defaultRatio, const double womenJobRate, const int peasantsPerLevel)
{
	amount /= (popType.getDependentRatio().value_or(defaultRatio) + womenJobRate);
	const double shortage = hireFromWorseJobs(amount, peasantsPerLevel);
	jobCounts[popType.getType()] += amount - shortage;
}

// pre: subsistenceUnitEmployment must contain peasants TODO(Gawquon): Maybe not?
// post: MarketJobs accumulates subStatePop, and an equal number of jobs are distributed
// Returns number of subsistence building levels filled.
double V3::MarketJobs::createPeasants(const std::map<std::string, int>& subsistenceUnitEmployment,
	 double defaultRatio,
	 double womenJobRate,
	 int arableLand,
	 int subStatePop,
	 const std::map<std::string, PopType>& popTypes)
{
	auto unitEmployment = subsistenceUnitEmployment;
	for (const auto& [job, amount]: manorHouseRoster) // Peasants create manor house jobs.
	{
		unitEmployment[job] += amount;
	}
	// const int totalUnitPop = std::accumulate(unitEmployment.begin(), unitEmployment.end(), 0, [](int sum, const auto& pair) {
	//	return sum + pair.second;
	// });



	double levels = getLevelsFromUnitEmploymentArable(unitEmployment, defaultRatio, womenJobRate, arableLand, subStatePop, popTypes);

	// TODO getLevelsFromUnitEmploymentArable
}

void V3::MarketJobs::loadInitialJobs(const std::map<std::string, double>& jobsList)
{
	jobCounts = jobsList;
	population = std::accumulate(jobCounts.begin(), jobCounts.end(), 0, [](int sum, const auto& pair) {
		return sum + static_cast<int>(pair.second);
	});
}

double V3::MarketJobs::hireFromWorseJobs(double amount, const int peasantsPerLevel)
{
	amount = hireFromUnemployed(amount);
	amount = hireFromPeasants(amount, peasantsPerLevel);
	return amount;
}

double V3::MarketJobs::hireFromUnemployed(double amount)
{
	const double unemployed = jobCounts.at("unemployed");
	if (unemployed > amount)
	{
		jobCounts["unemployed"] -= amount;
		return 0;
	}
	amount -= unemployed;
	jobCounts["unemployed"] = 0;
	return amount;
}

double V3::MarketJobs::hireFromPeasants(double amount, const int peasantsPerLevel)
{
	// When peasants are removed, Manor Houses downsize
	const double peasants = jobCounts.at("peasants");
	if (peasants > amount)
	{
		jobCounts["peasants"] -= amount;
		downsizeManorHouses(amount, peasantsPerLevel);
		return 0;
	}
	amount -= peasants;
	jobCounts["peasants"] = 0;
	downsizeManorHouses(peasants, peasantsPerLevel);
	return amount;
}

void V3::MarketJobs::downsizeManorHouses(const double peasantAmount, const int peasantsPerLevel)
{
	for (const auto& [job, amount]: manorHouseRoster)
	{
		jobCounts[job] -= amount * peasantAmount / peasantsPerLevel;
	}
}
