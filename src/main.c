#include "lib/lib.h"

int main(int argc, char **argv) {
    struct hashmap *c = init_corpus();
    char *query = "1929";
    char *doc1 = "The Tower Realty Company was approved to build on the site in 1929. By May 1929 the site was cleared by razing buildings which included the State Savings and Trust and the Franklin buildings.[1] There was also the four-story building of the Starr Piano Company on the site which had to be purchased and razed. The piano store's 99-year lease also had to be purchased by the Tower Realty Company";

    char *doc2 = "William Ellis Newton, VC (8 June 1919 – 29 March 1943) was an Australian recipient of the Victoria Cross, the highest decoration for gallantry in the face of the enemy that can be awarded to a member of the British and Commonwealth armed forces. He was honoured for his actions as a bomber pilot in Papua New Guinea during March 1943 when, despite intense anti-aircraft fire, he pressed home a series of attacks on the Salamaua Isthmus, the last of which saw him forced to ditch his aircraft in the sea. Newton was still officially posted as missing when the award was made in October 1943. It later emerged that he had been taken captive by the Japanese, and executed by beheading on 29 March.";

    add_or_update_document(c, "doc1", doc1);
    add_or_update_document(c, "doc2", doc2);
    size_t i = 0;
    void *corpus_item = NULL;
    while (hashmap_iter(c, &i, &corpus_item)) {
        struct Document *doc = corpus_item;
        size_t j = 0;
        void *doc_item = NULL;
        while (hashmap_iter(doc->terms ,&j, &doc_item)) {
            struct Term *term = doc_item;
        }
    }
    struct QueryResult *ans = search_query(c,query);
    for (int x = 0; x < dynarray_length(ans); ++x) {
        printf("result %s has score of %f \n", ans[x].key, ans[x].score);
    }
    free_query_results(ans, dynarray_length(ans));
    hashmap_free(c);
}
