#!/usr/bin/env python3

import os
import re
import nltk
import pickle
import random
import pathlib
from . import SpamfilterBase
from nltk.corpus import stopwords
from email import message_from_binary_file, policy
from nltk import NaiveBayesClassifier, word_tokenize

class NaiveBayes(SpamfilterBase):
    def __init__(self, spamdir, hamdir):
        self._del_chars = { ord(x): '' for x in "?!#%&" }
        self._spamdir = spamdir
        self._hamdir = hamdir

        # Try to load the classifier from .classifier file
        # (previously serialized classifier via pickle)
        # If the classifier does not exist, create it from
        # the train emails from spamdir and hamdir directories
        try:
            self._classifier = pickle.load(open(".classifier", "rb"))
        except:
            # Prepare the word dictionary
            spamdict = self._prepare_dictionary(self._spamdir)
            hamdict = self._prepare_dictionary(self._hamdir)

            # Prepare classifier
            combined_dict = [(words, "spam") for words in spamdict]
            combined_dict += [(words, "ham") for words in hamdict]
            random.shuffle(combined_dict)
            features = [(self._prepare_word_features(words), label) for (words, label) in combined_dict]
            self._classifier = NaiveBayesClassifier.train(features)

            # Serialize and save the classifier
            pickle.dump(self._classifier, open(".classifier", "wb"))

        #self._classifier.show_most_informative_features(50)

    def _process_data(self, data):
        # Remove special characters and stop words from the data set and
        # split it to tokens
        stopw = set(stopwords.words())
        tokens = [x.translate(self._del_chars) for x in word_tokenize(data.lower()) if x not in stopw]

        return tokens

    def _prepare_dictionary(self, email_dir):
        p = pathlib.Path(email_dir)
        emails = list(p.glob("**/*.*"))
        word_dict = []

        for em in emails:
            try:
                with open(em, "rb") as fp:
                    e = message_from_binary_file(fp, policy=policy.default)
                    body = e.get_body(preferencelist=("plain", "html")).get_content()
                    content = re.sub('<[^<]+?>', '', body)

                    words = self._process_data(content)
                    words += self._process_data(e.get("Subject"))
            except Exception as e:
                #print("[E]: {} ({})".format(e, em))
                continue

            word_dict.append(words)

        return word_dict

    def _prepare_word_features(self, words):
        return dict([(word, True) for word in words])

    def classify(self, em):
        try:
            body = em.get_body(preferencelist=("plain", "html")).get_content()
            words = self._process_data(body)
            words += self._process_data(em.get("Subject"))
            features = self._prepare_word_features(words)
            cl = self._classifier.classify(features)

            return True if cl == "spam" else False
        except Exception as e:
            #print(e)
            return False
