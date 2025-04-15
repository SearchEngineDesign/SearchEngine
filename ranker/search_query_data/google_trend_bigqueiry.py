import bigframes.pandas as bpd
query_or_table = "bigquery-public-data.google_trends.top_rising_terms"
bq_df = bpd.read_gbq(query_or_table)
csv_filename = "google_trends_top_rising_terms.csv"
bq_df.to_csv(csv_filename, index=False)