
#include "../../utils/ParsedUrl.h"
#include "../utils/string.h"

enum TopLevelDomains {
    COM,
    ORG,
    NET,
    EDU,
    GOV,
    IO,
    DOMAIN_COUNT
};

static float DOMAIN_WEIGHTS[DOMAIN_COUNT] = {1, 1, 1, 2, 2, 1};



class StaticRanker {
    
    private:
    float urlLengthWeight;
    float domainWeight;
        

    float getTopLevelDomain(const ParsedUrl& url) const {
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

    StaticRanker(float urlLengthWeight, float domainWeight) 
        : urlLengthWeight(urlLengthWeight), domainWeight(domainWeight) {}
    
    float rank(const ParsedUrl& url) const{
        if (url.urlName.empty()) return 0.0f;
        
        float rankScore = 0.0f;
        // rankScore += urlLengthWeight * url.urlName.length(); // Length of the URL
        rankScore += getTopLevelDomain(url); // Weight based on the top-level domain
    }
    
    
};





