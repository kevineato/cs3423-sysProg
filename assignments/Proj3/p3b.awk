BEGIN {
  printf("%-13s %-11s %-6s %s\n", "Name", "Flight", "Seats", "Total Cost")
}

/CUST/ {
  name = $6
}

/RESERVE/ {
  printf("%-13s %-11s %5d %11.2f\n", name, $2, $3, $3 * $4)
  custCost += ($3 * $4)

  if (flights[$2])
    flights[$2] += $3
  else
    flights[$2] = $3
}

/ENDCUST/ {
  printf("%-25s %s %11.2f\n", "", "Total", custCost)
  if (custCost) {
    custCost = 0.0
  }
}

END {
  printf("%-11s %s\n", "Flight", "Total Seats")
  for (flight in flights) {
    printf("%-17s %2d\n", flight, flights[flight])
  }
}
