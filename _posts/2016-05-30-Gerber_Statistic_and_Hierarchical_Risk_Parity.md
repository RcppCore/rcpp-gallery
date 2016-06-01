---
layout: post
title: "Gerber Statistic and Hierarchical Risk Parity with Factor Modelling"
output: html_document
author: "Yi Kang and Chris Chung"
---





_By Nico, Yi and Chris_

## Portfolio Allocation

This section allocates portfolio by incorporating the [Gerber Statistic](http://papers.ssrn.com/sol3/papers.cfm?abstract_id=2627803), Factor Modelling, and the [Hierarchical Risk Parity](http://papers.ssrn.com/sol3/papers.cfm?abstract_id=2708678) framework. The Gerber Statistic is aimed to improve the correlation/covariance matrix by filtering out data sets that do not move beyond a certain threshold. The HRP framework is used to allocate weightings by clustering securities based on their similarities and then enforce a risk parity among the clusters.

The steps to generate portfolio weightings for each period is as follows

- filter out the securities with available data for both security returns and factor returns
- apply cross-sectional regression on security returns to generate factor betas and specific risks
- use Gerber Statistic method to generate factor covariance matrix
- combine factor betas, factor covariance matrix, and specific risks to generate the covariance and the correlation matrix for the portfolio securities
- apply the HRP framework with covariance and correlation matrices as input, generating the weightings for each security in the portfolio
- benchmark the portfolio cumulative returns against benchmark

Three different portfolios were generated to showcase the results. The three portfolios are:

* Vanilla Covariance Matrix with HRP Generated Weights
* Gerber Statistic Covariance Matrix with Minimum Variance Weights
* Gerber Statistic Covariance Matrix with HRP Generated Weights

These portfolios were then compared to the S&P 500. The date of valuation starts from January 31, 2007 due to the limited time frame of the data.  

## Data Description

The universe of equities selected comrpise of equities listed in the S&P 500 and NASDAQ since 2005 adjusted for survivorship bias. The data is sourced from Bloomberg. The historical data for the equities stretches back to 1990 with the backtest only begining in 2005. This is due for the need to operate within the bloomberg monthly limit, the use of multi-year averages in the factors, and the need to measure the consecutive changes accurately by providing a long enough history for the computation to provide a realistic value. Companies with a market capitalization of less than $10 million are excluded from the analysis. 

The factors being analyzed are categorized into 7 groups. These include: 

1. Traditional Value (TV)

2. Relative Value (RV)

3. Historical Growth (HG)

4. Profit Trends (PT)

5. Price Momentum (PM)

6. Price Reversal (PR)

7. Small Size (SS)

For the composition of these factor groups, please see the appendix. 

Only equities that have the information to calculate all the factors in the factor groups are included in the analysis due to a lack of available data from bloomberg for every firm. This leaves us with 962 stocks at the start of the time series and ending with 1824 in the universe. 




<img src="http://nextlevelanalytics.github.io/public/images/2016-05-30-Gerber_Statistic_and_Hierarchical_Risk_Parity/unnamed-chunk-4-1.png" title="plot of chunk unnamed-chunk-4" alt="plot of chunk unnamed-chunk-4" style="display: block; margin: auto;" /><img src="http://nextlevelanalytics.github.io/public/images/2016-05-30-Gerber_Statistic_and_Hierarchical_Risk_Parity/unnamed-chunk-4-2.png" title="plot of chunk unnamed-chunk-4" alt="plot of chunk unnamed-chunk-4" style="display: block; margin: auto;" /><img src="http://nextlevelanalytics.github.io/public/images/2016-05-30-Gerber_Statistic_and_Hierarchical_Risk_Parity/unnamed-chunk-4-3.png" title="plot of chunk unnamed-chunk-4" alt="plot of chunk unnamed-chunk-4" style="display: block; margin: auto;" /><img src="http://nextlevelanalytics.github.io/public/images/2016-05-30-Gerber_Statistic_and_Hierarchical_Risk_Parity/unnamed-chunk-4-4.png" title="plot of chunk unnamed-chunk-4" alt="plot of chunk unnamed-chunk-4" style="display: block; margin: auto;" />


Here are the specific performances of each individual strategy:

### Vanilla Covariance and HRP

<img src="http://nextlevelanalytics.github.io/public/images/2016-05-30-Gerber_Statistic_and_Hierarchical_Risk_Parity/unnamed-chunk-5-1.png" title="plot of chunk unnamed-chunk-5" alt="plot of chunk unnamed-chunk-5" style="display: block; margin: auto;" />

### Gerber and HRP

<img src="http://nextlevelanalytics.github.io/public/images/2016-05-30-Gerber_Statistic_and_Hierarchical_Risk_Parity/unnamed-chunk-6-1.png" title="plot of chunk unnamed-chunk-6" alt="plot of chunk unnamed-chunk-6" style="display: block; margin: auto;" />

### Gerber and Minimum-Variance

<img src="http://nextlevelanalytics.github.io/public/images/2016-05-30-Gerber_Statistic_and_Hierarchical_Risk_Parity/unnamed-chunk-7-1.png" title="plot of chunk unnamed-chunk-7" alt="plot of chunk unnamed-chunk-7" style="display: block; margin: auto;" />

## S&P 500

<img src="http://nextlevelanalytics.github.io/public/images/2016-05-30-Gerber_Statistic_and_Hierarchical_Risk_Parity/unnamed-chunk-8-1.png" title="plot of chunk unnamed-chunk-8" alt="plot of chunk unnamed-chunk-8" style="display: block; margin: auto;" />



## Appendix

Here are the components of the factor groups:

1. Traditional Value (TV)

* Price/12-Month Forward Earnings Consensus Estimate. Twelve-month forward earnings is calculated as the time-weighted average of FY1 and FY2 (the upcoming and the following fiscal year-end earnings forecasts). The weight for FY1 is the ratio of the number of days left in the year to the total number of days in a year, and the weight for FY2 is one minus the weight for FY1.
* Price/Trailing 12-Month Sales. Trailing sales is computed as the sum of the quarterly sales over the last 4 quarters.
Price/Trailing 12-Month Cash Flows. The trailing cash flow is computed as the sum of the quarterly cash flow over the last 4 quarters.
* Dividend Yield. This is computed as the total DPS paid over the last year, divided by the current price.
* Price/Book Value. For the book value we use the last quarterly value.

2. Relative Value (RV)

* Industry-Relative Price/Trailing 12-Month Sales
* Industry-Relative Price/Trailing 12-Month Earnings
* Industry-Relative Price/Trailing 12-Month Cash Flows
* Industry-Relative Price/Trailing 12-Month Sales (Current Spread vs. 5-Year Average)
* Industry-Relative Price/Trailing 12-Month Earnings (Current Spread vs. 5-Year Average)
* Industry-Relative Price/Trailing 12-Month Cash Flows (Current Spread vs. 5-Year Average)

3. Historical Growth (HG)
* Number of Consecutive Quarters of Positive Changes in Trailing 12-Month Cash Flows (counted over the last 24 quarters). For each of the last 24 quarters we compute the trailing 12-month cash flow, and then count the number of times the consecutive changes in those trailing cash flows are of the same sign from quarter to quarter, starting with the most recent quarter and going back. If the consecutive quarter-to-quarter changes are negative, we count each change as -1, and if they are positive we count each change as +1.
* Number of Consecutive Quarters of Positive Change in Trailing 12-Month Quarterly Earnings (counted over the last 24 quarters). We calculate the trailing 12-month quarterly earnings by summing up the quarterly earnings for the last 4 quarters, and compute the number of consecutive quarters in the same way as in the item above.
* 12-Month Change in Quarterly Cash Flows. This is the difference between the trailing 12-month cash flow for the most recent quarter and the trailing 12-month cash flow for the quarter exactly one year back from the most recent quarter. 3-Year Average Annual Sales Growth. For each of the last 3 years we compute the 1-year percentage change in sales, and then compute the 3-year average of those 1-year percentage changes.
* 3-Year Average Annual Earnings Growth. The same calculation as in the item above is done, but for earnings. 
* 12-Quarter Trendline in Trailing 12-Month Earnings. For each of the last 12 quarters we take the trailing 12- month earnings and calculate the slope of the linear trendline fitted to those 12 points, and then divide that slope by the average 12-month trailing earnings across all 12 quarters.
* 12-Quarter Trendline in Trailing 12-Month Cash Flows. This is calculated in the same way as described in the item above, but using cash flows instead of earnings.

4. Profit Trends (PT)
* Number of Consecutive Quarters of Declines in (Receivables + Inventories)/Trailing 12-Month Sales (counted over the last 24 quarters). We start with the most recent quarter and count back. If the consecutive quarter-to-quarter changes are negative, we count each change as +1, and if they are positive we count each change as -1. Receivables is calculated as the average of the receivables for this quarter and the quarter one year ago, and the inventories number is calculated similarly.
Number of Consecutive Quarters of Positive Change in Trailing 12-Month Cash Flows/Trailing
* 12-Month Sales (counted over the last 24 quarters). We start with the most recent quarter and count back. If the consecutive quarter-toquarter changes are positive, we count each changeas +1, and if they are negative we count each change as -1.
* Consecutive Quarters of Declines in Trailing 12-Month Overhead/Trailing 12-Month Sales (counted over the last 24 quarters). We start with the most recent quarter and count back. If the consecutive quarter-to-quarter changes are negative, we count each change as +1, and if they are positive we count each change as -1. The trailing 12-month overhead equals trailing 12- month sales minus trailing 12-month COGS minus trailing 12-month EBEX, where the trailing 12-month values are obtained by summing the quarterly values for the last 4 quarters.
* Industry-Relative Trailing 12-Month (Receivables + Inventories)/Trailing 12-Month Sales. The industry-relative ratio is obtained by standardizing the underlying ratio using the mean and standard deviation of that ratio across all companies in that industry group.
* Industry-Relative Trailing 12-Month Sales/Assets. The assets value is the average of the assets for this quarter and the assets for the quarter one year ago. The industry-relative ratio is obtained by standardizing the underlying ratio using the mean and standard deviation of that ratio across all companies in that industry group.
* Trailing 12-Month Overhead/Trailing 12-Month Sales. Trailing 12-month overhead equals trailing 12-month sales minus trailing 12-month COGS minus trailing 12-month EBEX, where the trailing 12-month values are obtained by summing the quarterly values for the last 4 quarters.
* Trailing 12-Month Earnings/Trailing 12-Month Sales

5. Price Momentum (PM)
* Slope of 52-Week Trendline (calculated with 20-day lag) Percent Above 260-Day Low (calculated with 20-day lag)
* 4/52-Week Price Oscillator (calculated with 20-day lag). This is computed as the ratio of the average weekly price over the past 4 weeks to the average weekly price over the past 52 weeks, minus 1.
* 39-Week Return (calculated with 20-day lag)
* 52-Week Volume Price Trend (calculated with 20-day lag).

6. Price Reversal (PR)
* 5-Day Industry-Relative Return. This is calculated as the 5-day return minus the cap-weighted average 5-day return within that industry.
* 5-Day Money Flow/Volume. To obtain the numerator of this ratio, for each of the past 5 days we compute the closing price times the volume (shares traded) for that day, multiply that by -1 if that day's return is negative, and sum those daily values. To obtain the denominator, we simply sum the closing price times the daily volume across the past 5 days (without multiplying those daily products further by -1 if the corresponding daily return is negative).
* 12-26 Day MACD-10-Day Signal Line.
* 14-Day RSI (Relative-Strength Index).
* 20-Day Lane's Stochastic Indicator.
* 4-Week Industry-Relative Return. This is calculated as the 4-week return minus the capweighted average 4-week return within that industry.

7. Small Size (SS)
* Log of Market Capitalization
* Log of Market Capitalization Cubed
* Log of Stock Price
* Log of Total Last Quarter Assets
* Log of Trailing 12-Month Sales
