SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='';

DROP SCHEMA IF EXISTS `alder` ;
CREATE SCHEMA IF NOT EXISTS `alder` DEFAULT CHARACTER SET latin1 COLLATE latin1_swedish_ci ;
USE `alder` ;

-- -----------------------------------------------------
-- Table `alder`.`study`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `alder`.`study` ;

CREATE  TABLE IF NOT EXISTS `alder`.`study` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `update_timestamp` TIMESTAMP NOT NULL ,
  `created_timestamp` TIMESTAMP NOT NULL ,
  `uid` VARCHAR(45) NOT NULL ,
  `site` VARCHAR(45) NOT NULL ,
  `interviewer` VARCHAR(45) NOT NULL ,
  `datetime_acquired` DATETIME NOT NULL ,
  `datetime_reviewed` DATETIME NULL ,
  `note` BLOB NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `dk_uid` (`uid` ASC) ,
  INDEX `dk_datetime_reviewed` (`datetime_reviewed` ASC) ,
  INDEX `dk_site` (`site` ASC) ,
  INDEX `dk_datetime_acquired` (`datetime_acquired` ASC) ,
  INDEX `dk_interviewer` (`interviewer` ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `alder`.`series`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `alder`.`series` ;

CREATE  TABLE IF NOT EXISTS `alder`.`series` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `update_timestamp` TIMESTAMP NOT NULL ,
  `created_timestamp` TIMESTAMP NOT NULL ,
  `study_id` INT UNSIGNED NOT NULL ,
  `laterality` ENUM('right','left') NOT NULL ,
  `type` ENUM('cimt','plaque') NOT NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `fk_series_study_id` (`study_id` ASC) ,
  INDEX `dk_laterality` (`laterality` ASC) ,
  INDEX `dk_type` (`type` ASC) ,
  CONSTRAINT `fk_series_study_id`
    FOREIGN KEY (`study_id` )
    REFERENCES `alder`.`study` (`id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `alder`.`cineloop`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `alder`.`cineloop` ;

CREATE  TABLE IF NOT EXISTS `alder`.`cineloop` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `update_timestamp` TIMESTAMP NOT NULL ,
  `created_timestamp` TIMESTAMP NOT NULL ,
  `series_id` INT UNSIGNED NOT NULL ,
  `data` LONGBLOB NOT NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `fk_cineloop_series_id` (`series_id` ASC) ,
  CONSTRAINT `fk_cineloop_series_id`
    FOREIGN KEY (`series_id` )
    REFERENCES `alder`.`series` (`id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `alder`.`image`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `alder`.`image` ;

CREATE  TABLE IF NOT EXISTS `alder`.`image` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `update_timestamp` TIMESTAMP NOT NULL ,
  `created_timestamp` TIMESTAMP NOT NULL ,
  `series_id` INT UNSIGNED NOT NULL ,
  `cineloop_id` INT UNSIGNED NOT NULL ,
  `frame` INT NOT NULL ,
  `interviewer_defined` TINYINT(1) NOT NULL ,
  `min` FLOAT NULL ,
  `max` FLOAT NULL ,
  `mean` FLOAT NULL ,
  `sd` FLOAT NULL ,
  `n` INT(11) NULL ,
  `data` LONGBLOB NOT NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `fk_image_series_id` (`series_id` ASC) ,
  INDEX `fk_image_cineloop_id` (`cineloop_id` ASC) ,
  INDEX `dk_cineloop_id_frame` (`cineloop_id` ASC, `frame` ASC) ,
  CONSTRAINT `fk_image_series_id`
    FOREIGN KEY (`series_id` )
    REFERENCES `alder`.`series` (`id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_image_cineloop_id`
    FOREIGN KEY (`cineloop_id` )
    REFERENCES `alder`.`cineloop` (`id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `alder`.`user`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `alder`.`user` ;

CREATE  TABLE IF NOT EXISTS `alder`.`user` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `update_timestamp` TIMESTAMP NOT NULL ,
  `created_timestamp` TIMESTAMP NOT NULL ,
  `name` VARCHAR(255) NOT NULL ,
  `password` VARCHAR(255) NOT NULL ,
  `last_login` DATETIME NULL ,
  PRIMARY KEY (`id`) ,
  UNIQUE INDEX `uq_name` (`name` ASC) ,
  INDEX `dk_last_login` (`last_login` ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `alder`.`rating`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `alder`.`rating` ;

CREATE  TABLE IF NOT EXISTS `alder`.`rating` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `update_timestamp` TIMESTAMP NOT NULL ,
  `created_timestamp` TIMESTAMP NOT NULL ,
  `image_id` INT UNSIGNED NOT NULL ,
  `user_id` INT UNSIGNED NOT NULL ,
  `rating` TINYINT(1) NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `fk_rating_image_id` (`image_id` ASC) ,
  INDEX `fk_rating_user_id` (`user_id` ASC) ,
  INDEX `dk_rating` (`rating` ASC) ,
  CONSTRAINT `fk_rating_image_id`
    FOREIGN KEY (`image_id` )
    REFERENCES `alder`.`image` (`id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_rating_user_id`
    FOREIGN KEY (`user_id` )
    REFERENCES `alder`.`user` (`id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;



SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
