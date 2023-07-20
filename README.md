# CGPIO
# Program 8.2 - Percentage Closer Filtering (PCF)
There are various ways of simulating the penumbra effect to generate soft shadows in software. One of the simplest and most common is called Percentage Closer Filtering (PCF). In PCF, we sample the shadow texture at several surrounding locations to estimate what percentage of nearby locations are in shadow. Depending on how many of the nearby locations are in shadow, we increase or decrease the degree of lighting contribution for the pixel being rendered. PCF can also be used to reduce jagged line artifacts
