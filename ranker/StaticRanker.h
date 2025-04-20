
#include "../utils/ParsedUrl.h"
#include "../utils/searchstring.h"
#include <cmath>

enum TopLevelDomains {
    COM,
    ORG,
    NET,
    EDU,
    GOV,
    IO,
    DOMAIN_COUNT
};

static float DOMAIN_WEIGHTS[DOMAIN_COUNT] = {1, 1.2, 1, 1.3, 1.4, 0.8};

class StaticRanker {
    
    private:
    static constexpr float urlLengthWeight = 0;
    static constexpr float domainWeight = 1;
        

    static float getTopLevelDomain(const ParsedUrl& url) {
        if (url.Host.empty()) return COM; // Default to COM if Host is empty

        const string& tld = url.Domain;

        TopLevelDomains tldEnum = COM; // Default to COM

        if (tld == "com") tldEnum = COM;
        else if (tld == "org") tldEnum = ORG;
        else if (tld == "net") tldEnum = NET;
        else if (tld == "edu") tldEnum = EDU;
        else if (tld == "gov") tldEnum = GOV;
        else if (tld == "io") tldEnum = IO;

        
        return DOMAIN_WEIGHTS[tldEnum] * domainWeight;
    }


    public:
    StaticRanker() = default;

    // StaticRanker(float urlLengthWeight, float domainWeight) 
    //     : urlLengthWeight(urlLengthWeight), domainWeight(domainWeight) {}
    
    static float rank(const ParsedUrl& url) {
        if (url.urlName.empty()) return 0.0f;
        
        float rankScore = 0.0f;
        // rankScore += urlLengthWeight * url.urlName.length(); // Length of the URL
        rankScore += getTopLevelDomain(url); // Weight based on the top-level domain

        rankScore -= (log(url.Path.charcount('/')) / 2);

        rankScore -= log(url.urlName.length());

        return rankScore;
    }
    
    bool operator() (const string &url1, const string &url2) {
        return rank(ParsedUrl(url1)) > rank(ParsedUrl(url2));
    }

};